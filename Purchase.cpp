#include "Purchase.h"
#include "Database.h"
#include "Session.h"
#include "Inventory.h"
#include "InputHelper.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

/* ================= GENERATE ORDER ID ================= */
string generateOrderID(sql::Connection* conn) {
    auto rs = conn->createStatement()->executeQuery(
        "SELECT OrderID FROM orders ORDER BY OrderID DESC LIMIT 1"
    );

    int num = 1;
    if (rs->next())
        num = stoi(rs->getString("OrderID").substr(3)) + 1;

    delete rs;

    stringstream ss;
    ss << "ORD" << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= DISPLAY SUPPLIERS ================= */
void displaySuppliers() {
    sql::Connection* conn = connectDB();
    auto rs = conn->createStatement()->executeQuery(
        "SELECT SupplierID, SupplierName FROM supplier"
    );

    cout << "\n=========== SUPPLIERS ===========\n";
    cout << left << setw(12) << "SupplierID" << "Supplier Name\n";
    cout << "---------------------------------\n";

    while (rs->next()) {
        cout << left
            << setw(12) << rs->getString("SupplierID")
            << rs->getString("SupplierName") << endl;
    }

    delete rs;
    delete conn;
}

/* ================= DISPLAY INVENTORY BY CATEGORY ================= */
void displayInventoryByCategory(const string& category) {
    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT i.InventoryID, p.ProductName, i.quantity_in_stock "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode=p.ItemCode "
        "WHERE p.Category=? "
        "ORDER BY i.InventoryID"
    );
    ps->setString(1, category);

    auto rs = ps->executeQuery();

    cout << "\n--- INVENTORY (" << category << ") ---\n";
    cout << left
        << setw(10) << "InvID"
        << setw(25) << "Product"
        << setw(10) << "Stock\n";
    cout << "---------------------------------------------\n";

    while (rs->next()) {
        cout << left
            << setw(10) << rs->getString("InventoryID")
            << setw(25) << rs->getString("ProductName")
            << setw(10) << rs->getInt("quantity_in_stock") << endl;
    }

    delete rs;
    delete ps;
    delete conn;
}

void insertPurchase() {
    system("cls");
    cout << "=========== ADD PURCHASE ===========" << endl;

    /* ===== CATEGORY ===== */
    string category = chooseCategory();
    if (category.empty()) return;

    sql::Connection* conn = connectDB();

    try {
        /* ===== SELECT INVENTORY ===== */
        system("cls");
        displayInventoryByCategory(category);

        cout << "\nEnter InventoryID (0 Back): ";
        string inventoryID;
        if (!inputString(inventoryID) || inventoryID == "0") {
            delete conn;
            return;
        }

        /* ===== GET ITEMCODE ===== */
        string itemCode;
        auto psItem = conn->prepareStatement(
            "SELECT ItemCode FROM inventory WHERE InventoryID=?"
        );
        psItem->setString(1, inventoryID);
        auto rsItem = psItem->executeQuery();

        if (!rsItem->next()) {
            cout << "\n[ERROR] Inventory not found.\n";
            delete rsItem; delete psItem;
            delete conn;
            pauseScreen();
            return;
        }

        itemCode = rsItem->getString("ItemCode");
        delete rsItem; delete psItem;

        /* ===== CHECK SUPPLIER COUNT ===== */
        auto psCnt = conn->prepareStatement(
            "SELECT COUNT(DISTINCT o.SupplierID) AS cnt "
            "FROM orders o "
            "JOIN order_line ol ON o.OrderID = ol.OrderID "
            "WHERE ol.ItemCode=?"
        );
        psCnt->setString(1, itemCode);
        auto rsCnt = psCnt->executeQuery();

        int supplierCount = 0;
        if (rsCnt->next())
            supplierCount = rsCnt->getInt("cnt");

        delete rsCnt; delete psCnt;

        string supplierID;

        /* ===== NO PREVIOUS PURCHASE ===== */
        if (supplierCount == 0) {
            displaySuppliers();
            cout << "\nSelect SupplierID (0 Back): ";
            if (!inputString(supplierID) || supplierID == "0") {
                delete conn;
                return;
            }
        }

        /* ===== ONLY ONE SUPPLIER ===== */
        else if (supplierCount == 1) {
            auto psLast = conn->prepareStatement(
                "SELECT DISTINCT o.SupplierID "
                "FROM orders o "
                "JOIN order_line ol ON o.OrderID = ol.OrderID "
                "WHERE ol.ItemCode=? LIMIT 1"
            );
            psLast->setString(1, itemCode);
            auto rsLast = psLast->executeQuery();

            rsLast->next();
            string lastSupplier = rsLast->getString("SupplierID");

            delete rsLast; delete psLast;

            cout << "\nPreviously purchased from Supplier [" << lastSupplier << "]\n";
            cout << "Continue with same supplier? (Y/N): ";
            char c;
            cin >> c;
            cin.ignore(10000, '\n');

            if (c == 'Y' || c == 'y') {
                supplierID = lastSupplier;
            }
            else {
                displaySuppliers();
                cout << "\nSelect SupplierID (0 Back): ";
                if (!inputString(supplierID) || supplierID == "0") {
                    delete conn;
                    return;
                }
            }
        }

        /* ===== MULTIPLE SUPPLIERS ===== */
        else {
            cout << "\nItem previously purchased from multiple suppliers.\n";
            cout << "Please select a supplier:\n";
            displaySuppliers();

            cout << "\nSelect SupplierID (0 Back): ";
            if (!inputString(supplierID) || supplierID == "0") {
                delete conn;
                return;
            }
        }

        /* ===== CREATE ORDER HEADER ===== */
        string orderID = generateOrderID(conn);

        auto psOrder = conn->prepareStatement(
            "INSERT INTO orders "
            "(OrderID, Purchase_Date, Status, SupplierID, ActorID) "
            "VALUES (?, NOW(), 'Pending', ?, ?)"
        );
        psOrder->setString(1, orderID);
        psOrder->setString(2, supplierID);
        psOrder->setString(3, currentActorID);
        psOrder->execute();
        delete psOrder;

        cout << "\nOrder Created: " << orderID << endl;

        /* ===== ADD ITEMS ===== */
        char cont = 'Y';
        while (toupper(cont) == 'Y') {
            int qty;
            double price;

            cout << "\nQuantity (0 Back): ";
            if (!inputInt(qty) || qty == 0) break;

            cout << "Purchase Price: ";
            if (!inputDouble(price) || price <= 0) continue;

            auto psLine = conn->prepareStatement(
                "INSERT INTO order_line "
                "(OrderID, ItemCode, Quantity, UnitPrice) "
                "VALUES (?, ?, ?, ?)"
            );
            psLine->setString(1, orderID);
            psLine->setString(2, itemCode);
            psLine->setInt(3, qty);
            psLine->setDouble(4, price);
            psLine->execute();
            delete psLine;

            cout << "\nItem added successfully.\n";
            cout << "Add another item? (Y/N): ";
            cin >> cont;
            cin.ignore(10000, '\n');
        }

        cout << "\n=========== PURCHASE COMPLETED ===========" << endl;
        cout << "Order ID : " << orderID << endl;
        cout << "Status   : Pending\n";
    }
    catch (sql::SQLException& e) {
        cout << "\n[SQL ERROR] " << e.what() << endl;
    }

    delete conn;
    pauseScreen();
}



void updatePurchase() {
    system("cls");

    sql::Connection* conn = connectDB();

    auto rs = conn->createStatement()->executeQuery(
        "SELECT o.OrderID, o.Purchase_Date, o.Status, o.SupplierID "
        "FROM orders o "
        "WHERE o.Status='Pending' "
        "ORDER BY o.Purchase_Date DESC"
    );

    bool found = false;

    cout << "\n=========== PENDING PURCHASE ORDERS ===========\n";

    while (rs->next()) {
        found = true;
        string orderID = rs->getString("OrderID");

        cout << "\nOrderID : " << orderID << endl;
        cout << "Date    : " << rs->getString("Purchase_Date") << endl;
        cout << "Supplier: " << rs->getString("SupplierID") << endl;
        cout << "Status  : " << rs->getString("Status") << endl;
        cout << "---------------------------------------------\n";
        cout << left
            << setw(12) << "ItemCode"
            << setw(8) << "Qty"
            << setw(12) << "UnitPrice"
            << setw(12) << "Subtotal" << endl;

        auto psLine = conn->prepareStatement(
            "SELECT ItemCode, Quantity, UnitPrice "
            "FROM order_line WHERE OrderID=?"
        );
        psLine->setString(1, orderID);
        auto rsLine = psLine->executeQuery();

        double total = 0.0;
        while (rsLine->next()) {
            double sub = rsLine->getInt("Quantity") *
                rsLine->getDouble("UnitPrice");
            total += sub;

            cout << left
                << setw(12) << rsLine->getString("ItemCode")
                << setw(8) << rsLine->getInt("Quantity")
                << setw(12) << fixed << setprecision(2)
                << rsLine->getDouble("UnitPrice")
                << setw(12) << sub << endl;
        }

        cout << "---------------------------------------------\n";
        cout << "Order Total: RM " << fixed << setprecision(2) << total << endl;

        delete rsLine;
        delete psLine;
    }

    delete rs;

    if (!found) {
        cout << "\nNo pending orders.\n";
        delete conn;
        pauseScreen();
        return;
    }

    cout << "\nEnter OrderID to mark as RECEIVED (0 Back): ";
    string orderID;
    if (!inputString(orderID) || orderID == "0") {
        delete conn;
        return;
    }

    string supplierID;
    auto psSup = conn->prepareStatement(
        "SELECT SupplierID FROM orders WHERE OrderID=? AND Status='Pending'"
    );
    psSup->setString(1, orderID);
    auto rsSup = psSup->executeQuery();

    if (!rsSup->next()) {
        cout << "\n[ERROR] Invalid OrderID.\n";
        delete rsSup;
        delete psSup;
        delete conn;
        pauseScreen();
        return;
    }

    supplierID = rsSup->getString("SupplierID");
    delete rsSup;
    delete psSup;

   

    auto psLine = conn->prepareStatement(
        "SELECT ItemCode, Quantity FROM order_line WHERE OrderID=?"
    );
    psLine->setString(1, orderID);
    auto rsLine = psLine->executeQuery();

    while (rsLine->next()) {
        string itemCode = rsLine->getString("ItemCode");
        int qty = rsLine->getInt("Quantity");

        // 1️⃣ Try UPDATE existing inventory
        auto psUpdate = conn->prepareStatement(
            "UPDATE inventory "
            "SET Quantity_In_Stock = Quantity_In_Stock + ? "
            "WHERE ItemCode=?"
        );
        psUpdate->setInt(1, qty);
        psUpdate->setString(2, itemCode);

        int rowsAffected = psUpdate->executeUpdate();
        delete psUpdate;

        // 2️⃣ If item does NOT exist, INSERT new record
        if (rowsAffected == 0) {
            auto psInsert = conn->prepareStatement(
                "INSERT INTO inventory "
                "(ItemCode, Quantity_In_Stock, SupplierID) "
                "VALUES (?, ?, ?)"
            );
            psInsert->setString(1, itemCode);
            psInsert->setInt(2, qty);
            psInsert->setString(3, supplierID);
            psInsert->execute();
            delete psInsert;
        }
    }

    delete rsLine;
    delete psLine;

    // Update order status
    auto ps = conn->prepareStatement(
        "UPDATE orders "
        "SET Status='Received', Receive_Date=NOW() "
        "WHERE OrderID=? AND Status='Pending'"
    );
    ps->setString(1, orderID);
    ps->executeUpdate();
    delete ps;

    cout << "\nOrder RECEIVED and inventory updated successfully.\n";

    delete conn;
    pauseScreen();
}



void cancelPurchase() {
    system("cls");

    sql::Connection* conn = connectDB();
    auto rs = conn->createStatement()->executeQuery(
        "SELECT OrderID, Purchase_Date, SupplierID "
        "FROM orders WHERE Status='Pending' "
        "ORDER BY Purchase_Date DESC"
    );

    bool found = false;

    cout << "\n=========== PENDING PURCHASE ORDERS ===========\n";

    while (rs->next()) {
        found = true;
        string orderID = rs->getString("OrderID");

        cout << "\nOrderID : " << orderID << endl;
        cout << "Date    : " << rs->getString("Purchase_Date") << endl;
        cout << "Supplier: " << rs->getString("SupplierID") << endl;
        cout << "---------------------------------------------\n";

        auto psLine = conn->prepareStatement(
            "SELECT ItemCode, Quantity FROM order_line WHERE OrderID=?"
        );
        psLine->setString(1, orderID);
        auto rsLine = psLine->executeQuery();

        while (rsLine->next()) {
            cout << rsLine->getString("ItemCode")
                << " x" << rsLine->getInt("Quantity") << endl;
        }

        delete rsLine;
        delete psLine;
    }

    delete rs;

    if (!found) {
        cout << "\nNo pending orders.\n";
        delete conn;
        pauseScreen();
        return;
    }

    cout << "\nEnter OrderID to CANCEL (0 Back): ";
    string orderID;
    if (!inputString(orderID) || orderID == "0") {
        delete conn;
        return;
    }

    char confirm;
    cout << "Confirm cancel order " << orderID << "? (Y/N): ";
    cin >> confirm;
    cin.ignore(10000, '\n');

    if (confirm != 'Y' && confirm != 'y') {
        delete conn;
        return;
    }

    auto ps = conn->prepareStatement(
        "UPDATE orders SET Status='Cancelled' "
        "WHERE OrderID=? AND Status='Pending'"
    );
    ps->setString(1, orderID);
    ps->executeUpdate();
    delete ps;

    cout << "\nOrder cancelled successfully.\n";
    delete conn;
    pauseScreen();
}


void viewPurchaseReport() {
    system("cls");

    sql::Connection* conn = connectDB();
    auto rs = conn->createStatement()->executeQuery(
        "SELECT OrderID, Purchase_Date, Receive_Date, Status, SupplierID "
        "FROM orders ORDER BY Purchase_Date DESC"
    );

    cout << "\n=============== PURCHASE REPORT ===============\n";

    while (rs->next()) {
        string orderID = rs->getString("OrderID");

        cout << "\nOrderID : " << orderID << endl;
        cout << "Purchase: " << rs->getString("Purchase_Date") << endl;
        cout << "Receive : " << rs->getString("Receive_Date") << endl;
        cout << "Status  : " << rs->getString("Status") << endl;
        cout << "Supplier: " << rs->getString("SupplierID") << endl;
        cout << "---------------------------------------------\n";

        auto ps = conn->prepareStatement(
            "SELECT ItemCode, Quantity, UnitPrice "
            "FROM order_line WHERE OrderID=?"
        );
        ps->setString(1, orderID);
        auto rsLine = ps->executeQuery();

        double total = 0.0;
        while (rsLine->next()) {
            double sub = rsLine->getInt("Quantity") * rsLine->getDouble("UnitPrice");
            total += sub;

            cout << rsLine->getString("ItemCode")
                << " x" << rsLine->getInt("Quantity")
                << " RM " << fixed << setprecision(2) << sub << endl;
        }

        cout << "---------------------------------------------\n";
        cout << "Total: RM " << fixed << setprecision(2) << total << endl;

        delete rsLine;
        delete ps;
    }

    delete rs;
    delete conn;
    pauseScreen();
}

