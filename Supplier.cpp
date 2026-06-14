#include "Supplier.h"
#include "Database.h"
#include "InputHelper.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

/* ================= GENERATE SUPPLIER ID ================= */
string generateSupplierID(sql::Connection* conn) {
    auto rs = conn->createStatement()->executeQuery(
        "SELECT SupplierID FROM supplier ORDER BY SupplierID DESC LIMIT 1"
    );

    int num = 1;
    if (rs->next())
        num = stoi(rs->getString("SupplierID").substr(3)) + 1;

    delete rs;

    stringstream ss;
    ss << "SUP" << setw(2) << setfill('0') << num;
    return ss.str();
}

/* ================= VIEW SUPPLIERS ================= */
void viewSupplier() {
    system("cls");

    sql::Connection* conn = connectDB();
    auto rs = conn->createStatement()->executeQuery(
        "SELECT SupplierID, SupplierName, Phone_number, Address, Email FROM supplier"
    );

    cout << "\n=========== SUPPLIER LIST ===========\n";
    cout << left
        << setw(8) << "ID"
        << setw(25) << "Name"
        << setw(15) << "Phone"
        << setw(20) << "Address"
        << setw(25) << "Email" << endl;

    cout << string(95, '-') << endl;

    bool found = false;
    while (rs->next()) {
        found = true;
        cout << left
            << setw(8) << rs->getString("SupplierID")
            << setw(25) << rs->getString("SupplierName")
            << setw(15) << rs->getString("Phone_number")
            << setw(20) << rs->getString("Address")
            << setw(25) << rs->getString("Email") << endl;
    }

    if (!found)
        cout << "\nNo supplier found.\n";

    delete rs;
    delete conn;

    pauseScreen();   
}

/* ================= INSERT SUPPLIER ================= */
void insertSupplier() {
    system("cls");
    cout << "=== ADD NEW SUPPLIER ===\n";
    cout << "(Enter 0 to Back)\n\n";

    string name, phone, address, email;

    cout << "Supplier Name: ";
    cin >> ws;
    getline(cin, name);
    if (name == "0") return;

    cout << "Phone Number: ";
    cin >> phone;
    if (phone == "0") return;

    cout << "Address: ";
    cin >> ws;
    getline(cin, address);
    if (address == "0") return;

    cout << "Email: ";
    cin >> email;
    if (email == "0") return;

    sql::Connection* conn = connectDB();

    try {
        string supplierID = generateSupplierID(conn);

        auto ps = conn->prepareStatement(
            "INSERT INTO supplier "
            "(SupplierID, SupplierName, Phone_number, Address, Email) "
            "VALUES (?,?,?,?,?)"
        );
        ps->setString(1, supplierID);
        ps->setString(2, name);
        ps->setString(3, phone);
        ps->setString(4, address);
        ps->setString(5, email);
        ps->execute();

        delete ps;
        delete conn;

        cout << "\nSupplier added successfully.\n";
        cout << "Supplier ID: " << supplierID << endl;
    }
    catch (sql::SQLException& e) {
        cout << "[SQL ERROR] " << e.what() << endl;
        delete conn;
    }

    pauseScreen();
}

/* ================= UPDATE SUPPLIER ================= */
void updateSupplier() {
    while (true) {
        viewSupplier();

        cout << "\nEnter SupplierID to update (0 Back): ";
        string supplierID;
        cin >> supplierID;
        if (supplierID == "0") return;

        sql::Connection* conn = connectDB();
        auto chk = conn->prepareStatement(
            "SELECT SupplierID FROM supplier WHERE SupplierID=?"
        );
        chk->setString(1, supplierID);
        auto rs = chk->executeQuery();

        if (!rs->next()) {
            delete rs; delete chk; delete conn;
            cout << "\nSupplier not found.\n";
            pauseScreen();
            continue;
        }

        delete rs; delete chk;

        cout << "\n1. Name\n2. Phone\n3. Address\n4. Email\n0. Back\nChoice: ";
        int c;
        if (!inputInt(c) || c == 0) {
            delete conn;
            return;
        }

        string value;
        cout << "New value (0 Back): ";
        cin >> ws;
        getline(cin, value);
        if (value == "0") {
            delete conn;
            continue;
        }

        string field =
            (c == 1) ? "SupplierName" :
            (c == 2) ? "Phone_number" :
            (c == 3) ? "Address" :
            (c == 4) ? "Email" : "";

        if (field.empty()) {
            delete conn;
            continue;
        }

        auto ps = conn->prepareStatement(
            "UPDATE supplier SET " + field + "=? WHERE SupplierID=?"
        );
        ps->setString(1, value);
        ps->setString(2, supplierID);
        ps->execute();

        delete ps;
        delete conn;

        cout << "\nSupplier updated successfully.\n";
        pauseScreen();
        return;
    }
}

/* ================= DELETE SUPPLIER ================= */
void deleteSupplier() {
    while (true) {
        viewSupplier();

        cout << "\nEnter SupplierID to delete (0 Back): ";
        string supplierID;
        cin >> supplierID;
        if (supplierID == "0") return;

        cout << "Confirm delete? (Y/N): ";
        char c;
        cin >> c;
        if (c != 'Y' && c != 'y') return;

        sql::Connection* conn = connectDB();
        auto ps = conn->prepareStatement(
            "DELETE FROM supplier WHERE SupplierID=?"
        );
        ps->setString(1, supplierID);
        ps->execute();

        delete ps;
        delete conn;

        cout << "\nSupplier deleted successfully.\n";
        pauseScreen();
        return;
    }
}
