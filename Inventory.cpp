#include "Inventory.h"
#include "Database.h"
#include "InputHelper.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

/* ================= GENERATE INVENTORY ID ================= */
string generateInventoryID(sql::Connection* conn) {
    auto st = conn->createStatement();
    auto rs = st->executeQuery(
        "SELECT InventoryID FROM inventory ORDER BY InventoryID DESC LIMIT 1"
    );

    int num = 1;
    if (rs->next()) {
        string lastID = rs->getString("InventoryID");
        num = stoi(lastID.substr(3)) + 1;
    }

    delete rs;
    delete st;

    stringstream ss;
    ss << "INV" << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= CHOOSE CATEGORY ================= */
string chooseCategory() {
    int choice;

    while (true) {
        cout << "Select Category:\n";
        cout << "1. Personal Care\n";
        cout << "2. Health & Beauty\n";
        cout << "3. Household Essentials\n";
        cout << "4. Toiletries\n";
        cout << "0. Back\n";
        cout << "Choice: ";

        if (!inputInt(choice)) return "";

        switch (choice) {
        case 1: return "Personal Care";
        case 2: return "Health & Beauty";
        case 3: return "Household Essentials";
        case 4: return "Toiletries";
        default:
            cout << "Invalid category.\n";
        }
    }
}

/* ================= GENERATE ITEM CODE ================= */
string generateItemCode(sql::Connection* conn, const string& category, const string& productName) {
    auto chk = conn->prepareStatement(
        "SELECT ItemCode FROM product WHERE ProductName=? LIMIT 1"
    );
    chk->setString(1, productName);

    auto rsChk = chk->executeQuery();
    if (rsChk->next()) {
        string code = rsChk->getString("ItemCode");
        delete rsChk;
        delete chk;
        return code;
    }

    delete rsChk;
    delete chk;

    string prefix;
    if (category == "Personal Care") prefix = "PC";
    else if (category == "Health & Beauty") prefix = "HB";
    else if (category == "Household Essentials") prefix = "HE";
    else if (category == "Toiletries") prefix = "TO";
    else prefix = "IT";

    auto ps = conn->prepareStatement(
        "SELECT ItemCode FROM product "
        "WHERE Category=? AND ItemCode LIKE ? "
        "ORDER BY ItemCode DESC LIMIT 1"
    );
    ps->setString(1, category);
    ps->setString(2, prefix + "%");

    auto rs = ps->executeQuery();

    int num = 1;
    if (rs->next()) {
        num = stoi(rs->getString("ItemCode").substr(2)) + 1;
    }

    delete rs;
    delete ps;

    stringstream ss;
    ss << prefix << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= GENERATE BATCH NUMBER ================= */
string generateBatchNumber(sql::Connection* conn, const string& category) {
    string prefix;

    if (category == "Personal Care") prefix = "PC";
    else if (category == "Health & Beauty") prefix = "HB";
    else if (category == "Household Essentials") prefix = "HE";
    else if (category == "Toiletries") prefix = "TO";
    else prefix = "XX";

    auto ps = conn->prepareStatement(
        "SELECT BatchNumber FROM inventory "
        "WHERE BatchNumber LIKE ? "
        "ORDER BY BatchNumber DESC LIMIT 1"
    );
    ps->setString(1, prefix + "-B%");

    auto rs = ps->executeQuery();

    int num = 1;
    if (rs->next()) {
        string lastBatch = rs->getString("BatchNumber");
        num = stoi(lastBatch.substr(4)) + 1;
    }

    delete rs;
    delete ps;

    stringstream ss;
    ss << prefix << "-B" << setw(3) << setfill('0') << num;
    return ss.str();
}

void displayInventoryList() {
    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT i.InventoryID, i.ItemCode, p.Category, p.ProductName, "
        "i.BatchNumber, i.quantity_in_stock, i.expiredate, i.SupplierID "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode = p.ItemCode "
        "ORDER BY p.Category, CAST(SUBSTRING(i.InventoryID, 4) AS UNSIGNED)"
    );

    auto rs = ps->executeQuery();

    string currentCategory = "";

    while (rs->next()) {
        string category = rs->getString("Category");

        // ===== NEW CATEGORY HEADER =====
        if (category != currentCategory) {
            currentCategory = category;

            cout << "\n" << category << "\n";
            cout << string(category.length(), '=') << "\n\n";

            // column header for each category
            cout << left
                << setw(8) << "ID"
                << setw(8) << "Code"
                << setw(20) << "Product"
                << setw(14) << "Batch"
                << setw(8) << "Qty"
                << setw(12) << "Expiry"
                << setw(10) << "Supplier"
                << endl;

            cout << string(90, '-') << endl;
        }

        // ===== ITEM ROW =====
        cout << left
            << setw(8) << rs->getString("InventoryID")
            << setw(8) << rs->getString("ItemCode")
            << setw(20) << rs->getString("ProductName")
            << setw(14) << rs->getString("BatchNumber")
            << setw(8) << rs->getInt("quantity_in_stock")
            << setw(12) << rs->getString("expiredate")
            << setw(10) << rs->getString("SupplierID")
            << endl;
    }

    delete rs;
    delete ps;
    delete conn;
}

/* ================= VIEW INVENTORY ================= */
void viewInventory() {
    system("cls");

    string category = chooseCategory();
    if (category.empty()) return;

    system("cls");

    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT i.InventoryID, i.ItemCode, p.Category, p.ProductName, "
        "i.BatchNumber, i.quantity_in_stock, i.expiredate, i.SupplierID "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode = p.ItemCode "
        "WHERE p.Category=?"
    );
    ps->setString(1, category);

    auto rs = ps->executeQuery();

    cout << "\n=========== INVENTORY (" << category << ") ===========\n";
    cout << left
        << setw(8) << "ID"
        << setw(8) << "Code"
        << setw(22) << "Category"
        << setw(20) << "Product"
        << setw(14) << "Batch"
        << setw(8) << "Qty"
        << setw(12) << "Expiry"
        << setw(10) << "Supplier"
        << endl;

    cout << string(120, '-') << endl;

    bool found = false;
    while (rs->next()) {
        found = true;
        cout << left
            << setw(8) << rs->getString("InventoryID")
            << setw(8) << rs->getString("ItemCode")
            << setw(22) << rs->getString("Category")
            << setw(20) << rs->getString("ProductName")
            << setw(14) << rs->getString("BatchNumber")
            << setw(8) << rs->getInt("quantity_in_stock")
            << setw(12) << rs->getString("expiredate")
            << setw(10) << rs->getString("SupplierID")
            << endl;
    }

    if (!found) {
        cout << "\n[INFO] No inventory records found in this category.\n";
    }

    delete rs;
    delete ps;
    delete conn;

    
    pauseScreen();
}



void insertInventory() {

    system("cls");

    string category = chooseCategory();
    if (category.empty()) return;

    system("cls");

    string product, expiry, supplier;
    int qty;
    double sellPrice;

    showBackInstruction();
    cout << "Product Name: ";
    if (!inputString(product)) return;

    while (true) {
        cout << "Selling Price (RM): ";
        if (!inputDouble(sellPrice)) return;
        if (sellPrice > 0) break;
        cout << "[ERROR] Invalid selling price.\n";
    }

    while (true) {
        cout << "Quantity: ";
        if (!inputInt(qty)) return;
        if (qty > 0) break;
        cout << "[ERROR] Invalid quantity.\n";
    }

    cout << "Expire Date (YYYY-MM-DD or NULL): ";
    if (!inputString(expiry)) return;

    cout << "Supplier ID: ";
    if (!inputString(supplier)) return;

    sql::Connection* conn = connectDB();

    try {
        auto chkSup = conn->prepareStatement(
            "SELECT SupplierID FROM supplier WHERE SupplierID=?"
        );
        chkSup->setString(1, supplier);
        auto rsSup = chkSup->executeQuery();

        if (!rsSup->next()) {
            cout << "\n[ERROR] Supplier ID not found.\n";
            pauseScreen();
            delete rsSup; delete chkSup; delete conn;
            return;
        }
        delete rsSup; delete chkSup;

        string invID = generateInventoryID(conn);
        string code = generateItemCode(conn, category, product);
        string batch = generateBatchNumber(conn, category);

        auto chkProd = conn->prepareStatement(
            "SELECT ItemCode FROM product WHERE ItemCode=?"
        );
        chkProd->setString(1, code);
        auto rsChk = chkProd->executeQuery();

        if (!rsChk->next()) {
            auto psProd = conn->prepareStatement(
                "INSERT INTO product (ItemCode, ProductName, Category, SellPrice) "
                "VALUES (?, ?, ?, ?)"
            );
            psProd->setString(1, code);
            psProd->setString(2, product);
            psProd->setString(3, category);
            psProd->setDouble(4, sellPrice);
            psProd->execute();
            delete psProd;
        }
        delete rsChk; delete chkProd;

        auto psInv = conn->prepareStatement(
            "INSERT INTO inventory "
            "(InventoryID, ItemCode, BatchNumber, quantity_in_stock, expiredate, SupplierID) "
            "VALUES (?, ?, ?, ?, ?, ?)"
        );
        psInv->setString(1, invID);
        psInv->setString(2, code);
        psInv->setString(3, batch);
        psInv->setInt(4, qty);
        if (expiry == "NULL")
            psInv->setNull(5, sql::DataType::DATE);
        else
            psInv->setString(5, expiry);
        psInv->setString(6, supplier);
        psInv->execute();
        delete psInv;

        cout << "\n Inventory Added Successfully!\n";
        cout << "InventoryID : " << invID << endl;
        cout << "ItemCode    : " << code << endl;
        cout << "BatchNumber : " << batch << endl;
        cout << fixed << setprecision(2);
        cout << "Sell Price  : RM " << sellPrice << endl;

        pauseScreen();
    }
    catch (sql::SQLException& e) {
        cout << "\n[SQL ERROR] " << e.what() << endl;
        pauseScreen();
    }

    delete conn;
}

void updateInventory() {
    system("cls");
    displayInventoryList();

    showBackInstruction();
    cout << "Enter Inventory ID to update: ";

    string invID;
    if (!inputString(invID)) return;

    sql::Connection* conn = connectDB();

    // Check Inventory ID existence
    auto chk = conn->prepareStatement(
        "SELECT 1 FROM inventory WHERE InventoryID=?"
    );
    chk->setString(1, invID);
    auto rs = chk->executeQuery();

    if (!rs->next()) {
        cout << "\n[ERROR] Inventory ID not found.\n";
        pauseScreen();
        delete rs;
        delete chk;
        delete conn;
        return;
    }
    delete rs;
    delete chk;

    system("cls");
    int choice;
    cout << "1. Update Quantity\n";
    cout << "2. Update Expiry Date\n";
    cout << "0. Back\n";
    cout << "Choice: ";

    if (!inputInt(choice) || choice == 0) {
        delete conn;
        return;
    }

    bool updated = false;

    if (choice == 1) {
        int qty;
        cout << "New Quantity: ";
        if (!inputInt(qty) || qty <= 0) {
            cout << "\n[ERROR] Invalid quantity.\n";
            pauseScreen();
            delete conn;
            return;
        }

        auto ps = conn->prepareStatement(
            "UPDATE inventory SET quantity_in_stock=? WHERE InventoryID=?"
        );
        ps->setInt(1, qty);
        ps->setString(2, invID);
        ps->executeUpdate();
        delete ps;

        updated = true;
    }
    else if (choice == 2) {
        string exp;
        cout << "New Expiry Date (YYYY-MM-DD or NULL): ";
        if (!inputString(exp)) {
            delete conn;
            return;
        }

        auto ps = conn->prepareStatement(
            "UPDATE inventory SET expiredate=? WHERE InventoryID=?"
        );
        if (exp == "NULL")
            ps->setNull(1, sql::DataType::DATE);
        else
            ps->setString(1, exp);

        ps->setString(2, invID);
        ps->executeUpdate();
        delete ps;

        updated = true;
    }
    else {
        cout << "\n[ERROR] Invalid choice. Please select 1 or 2.\n";
        pauseScreen();
        delete conn;
        return;
    }

    if (updated) {
        cout << "\nInventory updated successfully.\n";
        pauseScreen();
    }

    delete conn;
}



void deleteInventory() {
    system("cls");
    displayInventoryList();

    showBackInstruction();
    cout << "Enter Inventory ID to delete: ";

    string invID;
    if (!inputString(invID)) return;

    sql::Connection* conn = connectDB();

    auto ps = conn->prepareStatement(
        "DELETE FROM inventory WHERE InventoryID=?"
    );
    ps->setString(1, invID);
    int rows = ps->executeUpdate();
    delete ps;

    if (rows == 0)
        cout << "\n[ERROR] Inventory ID not found.\n";
    else
        cout << "\nInventory deleted successfully.\n";

    pauseScreen();
    delete conn;
}
