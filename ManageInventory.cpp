#include "ManageInventory.h"
#include "Purchase.h"
#include "Database.h"
#include "InputHelper.h"
#include "Session.h"

#include <iostream>
#include <iomanip>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

/* ================= MAIN INVENTORY MENU ================= */
void manageInventoryMenu() {
    int choice;
    do {
        system("cls");
        cout << "=== MANAGE INVENTORY MENU ===\n";
        cout << "1. Check Inventory Level (Reorder)\n";
        cout << "2. Check Inventory Expiry\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            checkInventoryLevel();
            break;
        case 2:
            checkInventoryExpire();   
            break;
        case 0:
            break;
        default:
            cout << "Invalid choice.\n";
            pauseScreen();
        }
    } while (choice != 0);

}

/* ================= SIMPLE SUPPLIER LIST ================= */
void displaySuppliersSimple() {
    sql::Connection* conn = connectDB();
    auto rs = conn->createStatement()->executeQuery(
        "SELECT SupplierID, SupplierName FROM supplier"
    );

    cout << "\n=========== SUPPLIER LIST ===========\n";
    cout << left << setw(10) << "ID" << "Name\n";
    cout << "-----------------------------------\n";

    while (rs->next()) {
        cout << left
            << setw(10) << rs->getString("SupplierID")
            << rs->getString("SupplierName") << endl;
    }

    delete rs;
    delete conn;
}

/* ================= LAST SUPPLIER ================= */
string getLastSupplierForItem(const string& itemCode) {
    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT o.SupplierID "
        "FROM orders o "
        "JOIN order_line ol ON o.OrderID = ol.OrderID "
        "WHERE ol.ItemCode=? "
        "ORDER BY o.Purchase_Date DESC "
        "LIMIT 1"
    );
    ps->setString(1, itemCode);

    auto rs = ps->executeQuery();
    string supplierID = "";

    if (rs->next())
        supplierID = rs->getString("SupplierID");

    delete rs;
    delete ps;
    return supplierID;
}

/* ================= REORDER LOGIC ================= */
void reorderLowStockItems() {
    sql::Connection* conn = connectDB();

    auto rsLow = conn->createStatement()->executeQuery(
        "SELECT InventoryID, ItemCode, quantity_in_stock "
        "FROM inventory WHERE quantity_in_stock < 10"
    );

    bool found = false;

    while (rsLow->next()) {
        found = true;

        string inventoryID = rsLow->getString("InventoryID");
        string itemCode = rsLow->getString("ItemCode");
        int currentQty = rsLow->getInt("quantity_in_stock");

        cout << "\nItemCode: " << itemCode
            << " | Current Qty: " << currentQty << endl;

        cout << "Reorder this item? (Y/N): ";
        char yn;
        cin >> yn;
        cin.ignore(10000, '\n');

        if (yn != 'Y' && yn != 'y')
            continue;

        int reorderQty;
        cout << "Enter reorder quantity: ";
        cin >> reorderQty;
        cin.ignore(10000, '\n');

        if (reorderQty <= 0) {
            cout << "Invalid quantity. Skipped.\n";
            continue;
        }

        string supplierID = getLastSupplierForItem(itemCode);


        if (!supplierID.empty()) {
            cout << "Last Supplier: " << supplierID << endl;
            cout << "Use same supplier? (Y/N): ";
            char same;
            cin >> same;
            cin.ignore(10000, '\n');

            if (same != 'Y' && same != 'y') {
                displaySuppliersSimple();
                cout << "Select SupplierID: ";
                cin >> supplierID;
            }
        }
        else {
            displaySuppliersSimple();
            cout << "Select SupplierID: ";
            cin >> supplierID;
        }

        string orderID = "";
        auto psChk = conn->prepareStatement(
            "SELECT OrderID FROM orders "
            "WHERE SupplierID=? AND Status='Pending' LIMIT 1"
        );
        psChk->setString(1, supplierID);
        auto rsChk = psChk->executeQuery();

        if (rsChk->next())
            orderID = rsChk->getString("OrderID");

        delete rsChk;
        delete psChk;

        if (orderID.empty()) {
            orderID = generateOrderID(conn);

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
        }


        auto psChkLine = conn->prepareStatement(
            "SELECT Quantity FROM order_line WHERE OrderID=? AND ItemCode=?"
        );
        psChkLine->setString(1, orderID);
        psChkLine->setString(2, itemCode);
        auto rsLine = psChkLine->executeQuery();

        if (rsLine->next()) {
            int oldQty = rsLine->getInt("Quantity");
            auto psUpd = conn->prepareStatement(
                "UPDATE order_line SET Quantity=? WHERE OrderID=? AND ItemCode=?"
            );
            psUpd->setInt(1, oldQty + reorderQty);
            psUpd->setString(2, orderID);
            psUpd->setString(3, itemCode);
            psUpd->execute();
            delete psUpd;
        }
        else {
            auto psLine = conn->prepareStatement(
                "INSERT INTO order_line (OrderID, ItemCode, Quantity, UnitPrice) "
                "VALUES (?, ?, ?, 0)"
            );
            psLine->setString(1, orderID);
            psLine->setString(2, itemCode);
            psLine->setInt(3, reorderQty);
            psLine->execute();
            delete psLine;
        }

        delete rsLine;
        delete psChkLine;

        cout << " Reorder placed under Order " << orderID << endl;
    }

    delete rsLow;
    delete conn;

    if (!found)
        cout << "\nNo low stock items found.\n";

    pauseScreen();
}

/* ================= CHECK INVENTORY LEVEL ================= */
void checkInventoryLevel() {
    system("cls");

    sql::Connection* conn = connectDB();
    auto rs = conn->createStatement()->executeQuery(
        "SELECT i.InventoryID, p.ProductName, i.quantity_in_stock "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode = p.ItemCode "
        "WHERE i.quantity_in_stock < 10 "
        "ORDER BY i.quantity_in_stock ASC"
    );

    cout << "\n--- LOW STOCK INVENTORY (Qty < 10) ---\n";
    cout << left
        << setw(12) << "InventoryID"
        << setw(25) << "Product Name"
        << setw(10) << "Qty"
        << "Status\n";

    cout << string(60, '-') << endl;

    bool found = false;
    while (rs->next()) {
        found = true;
        cout << left
            << setw(12) << rs->getString("InventoryID")
            << setw(25) << rs->getString("ProductName")
            << setw(10) << rs->getInt("quantity_in_stock")
            << "REORDER REQUIRED\n";
    }

    delete rs;
    delete conn;

    if (!found) {
        cout << "\nAll inventory levels are sufficient.\n";
        pauseScreen();
        return;
    }

    cout << "\nDo you want to reorder items now?\n";
    cout << "1. Select items to reorder\n";
    cout << "0. Back\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;
    cin.ignore(10000, '\n');

    if (choice == 1)
        reorderLowStockItems();
}

/* ================= CHECK EXPIRED & EXPIRING INVENTORY ================= */
void checkInventoryExpire() {

    sql::Connection* conn = connectDB();
    sql::Statement* st = conn->createStatement();

    /* ================= EXPIRED ITEMS ================= */
    cout << "\n--- EXPIRED INVENTORY ITEMS ---\n";

    sql::ResultSet* rs = st->executeQuery(
        "SELECT i.InventoryID, i.ItemCode, p.ProductName, "
        "i.expiredate, i.quantity_in_stock "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode = p.ItemCode "
        "WHERE i.expiredate IS NOT NULL "
        "AND i.expiredate < CURDATE() "
        "ORDER BY i.expiredate"
    );

    cout << left
        << setw(12) << "InventoryID"
        << setw(25) << "Product Name"
        << setw(15) << "Expire Date"
        << setw(10) << "Qty"
        << "Status\n";

    cout << string(75, '-') << endl;

    bool foundExpired = false;

    while (rs->next()) {
        foundExpired = true;

        string invID = rs->getString("InventoryID");
        string itemCode = rs->getString("ItemCode");
        string name = rs->getString("ProductName");
        string expDate = rs->getString("expiredate");
        int qty = rs->getInt("quantity_in_stock");

        cout << left
            << setw(12) << invID
            << setw(25) << name
            << setw(15) << expDate
            << setw(10) << qty
            << "EXPIRED\n";

        int choice;
        cout << "Action for " << invID << ":\n";
        cout << "1. RETURN (supplier replacement)\n";
        cout << "2. REMOVE (discard)\n";
        cout << "0. SKIP\n";
        cout << "Choice: ";
        cin >> choice;

        if (choice == 1) {
            /* ===== RETURN & REPLACE ===== */
            string newExp;
            cout << "Enter NEW expiry date (YYYY-MM-DD): ";
            cin >> newExp;

            // Delete old inventory
            auto del = conn->prepareStatement(
                "DELETE FROM inventory WHERE InventoryID=?"
            );
            del->setString(1, invID);
            del->execute();
            delete del;

            // Generate new InventoryID
            auto rsID = conn->createStatement()->executeQuery(
                "SELECT InventoryID FROM inventory ORDER BY InventoryID DESC LIMIT 1"
            );

            int num = 1;
            if (rsID->next()) {
                string lastID = rsID->getString(1);
                num = stoi(lastID.substr(3)) + 1;
            }
            delete rsID;

            string newInvID =
                "INV" + string(4 - to_string(num).length(), '0') + to_string(num);

            // Insert replacement inventory
            auto ins = conn->prepareStatement(
                "INSERT INTO inventory "
                "(InventoryID, ItemCode, quantity_in_stock, expiredate) "
                "VALUES (?, ?, ?, ?)"
            );
            ins->setString(1, newInvID);
            ins->setString(2, itemCode);
            ins->setInt(3, qty);
            ins->setString(4, newExp);
            ins->execute();
            delete ins;

            cout << "Returned and replaced with new inventory: "
                << newInvID << endl;
        }
        else if (choice == 2) {
            /* ===== REMOVE ===== */
            auto del = conn->prepareStatement(
                "DELETE FROM inventory WHERE InventoryID=?"
            );
            del->setString(1, invID);
            del->execute();
            delete del;

            cout << "Removed from inventory.\n";
        }
        else {
            cout << "Skipped.\n";
        }

        cout << endl;
    }

    delete rs;

    if (!foundExpired) {
        cout << "No expired inventory items.\n";
    }

    /* ================= EXPIRING SOON (30 DAYS) ================= */
    cout << "\n--- INVENTORY EXPIRING WITHIN 30 DAYS ---\n";

    rs = st->executeQuery(
        "SELECT i.InventoryID, i.ItemCode, p.ProductName, "
        "i.expiredate, i.quantity_in_stock "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode = p.ItemCode "
        "WHERE i.expiredate IS NOT NULL "
        "AND i.expiredate BETWEEN CURDATE() "
        "AND DATE_ADD(CURDATE(), INTERVAL 30 DAY) "
        "ORDER BY i.expiredate"
    );

    cout << left
        << setw(12) << "InventoryID"
        << setw(25) << "Product Name"
        << setw(15) << "Expire Date"
        << setw(10) << "Qty"
        << "Status\n";

    cout << string(75, '-') << endl;

    bool foundSoon = false;

    while (rs->next()) {
        foundSoon = true;

        string invID = rs->getString("InventoryID");
        string itemCode = rs->getString("ItemCode");
        string name = rs->getString("ProductName");
        string expDate = rs->getString("expiredate");
        int qty = rs->getInt("quantity_in_stock");

        cout << left
            << setw(12) << invID
            << setw(25) << name
            << setw(15) << expDate
            << setw(10) << qty
            << "EXPIRING SOON\n";

        int choice;
        cout << "Action for " << invID << ":\n";
        cout << "1. RETURN \n";
        cout << "0. SKIP\n";
        cout << "Choice: ";
        cin >> choice;

        if (choice == 1) {
            string newExp;
            cout << "Enter NEW expiry date (YYYY-MM-DD): ";
            cin >> newExp;

            auto del = conn->prepareStatement(
                "DELETE FROM inventory WHERE InventoryID=?"
            );
            del->setString(1, invID);
            del->execute();
            delete del;

            auto rsID = conn->createStatement()->executeQuery(
                "SELECT InventoryID FROM inventory ORDER BY InventoryID DESC LIMIT 1"
            );

            int num = 1;
            if (rsID->next()) {
                string lastID = rsID->getString(1);
                num = stoi(lastID.substr(3)) + 1;
            }
            delete rsID;

            string newInvID =
                "INV" + string(4 - to_string(num).length(), '0') + to_string(num);

            auto ins = conn->prepareStatement(
                "INSERT INTO inventory "
                "(InventoryID, ItemCode, quantity_in_stock, expiredate) "
                "VALUES (?, ?, ?, ?)"
            );
            ins->setString(1, newInvID);
            ins->setString(2, itemCode);
            ins->setInt(3, qty);
            ins->setString(4, newExp);
            ins->execute();
            delete ins;

            cout << "Returned and replaced with new inventory: "
                << newInvID << endl;
        }
        else {
            cout << "Skipped.\n";
        }

        cout << endl;
    }

    delete rs;

    if (!foundSoon) {
        cout << "No inventory expiring within 30 days.\n";
    }

    delete st;
    delete conn;
}


