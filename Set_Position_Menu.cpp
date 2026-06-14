#include "Set_Position_Menu.h"
#include "Database.h"

#include <iostream>
#include <iomanip>
#include <sstream>


#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

/* ================= GENERATE ADMIN ID ================= */
string generateAdminID(sql::Connection* conn) {
    auto st = conn->createStatement();
    auto rs = st->executeQuery(
        "SELECT AdminId FROM admin ORDER BY AdminId DESC LIMIT 1"
    );

    int num = 1;
    if (rs->next())
        num = stoi(rs->getString("AdminId").substr(1)) + 1;

    delete rs;
    delete st;

    stringstream ss;
    ss << "A" << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= GENERATE STAFF ID ================= */
string generateStaffID(sql::Connection* conn) {
    auto st = conn->createStatement();
    auto rs = st->executeQuery(
        "SELECT StaffID FROM staff ORDER BY StaffID DESC LIMIT 1"
    );

    int num = 1;
    if (rs->next())
        num = stoi(rs->getString("StaffID").substr(1)) + 1;

    delete rs;
    delete st;

    stringstream ss;
    ss << "E" << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= GENERATE ACTOR ID ================= */
string generateActorID(sql::Connection* conn, const string& prefix) {
    auto ps = conn->prepareStatement(
        "SELECT ActorID FROM actor WHERE ActorID LIKE ? ORDER BY ActorID DESC LIMIT 1"
    );
    ps->setString(1, prefix + "%");

    auto rs = ps->executeQuery();

    int num = 1;
    if (rs->next())
        num = stoi(rs->getString("ActorID").substr(2)) + 1;

    delete rs;
    delete ps;

    stringstream ss;
    ss << prefix << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= SET POSITION MENU ================= */
void setPositionMenu() {
    int choice;
    do {
        system("cls");
        cout << "=== SET POSITION MENU ===\n";
        cout << "1. Display Admin & Staff\n";
        cout << "2. Add Admin / Staff\n";
        cout << "3. Remove Admin / Staff\n";
        cout << "4. Update Admin / Staff\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
        case 1: listAdminStaff(); break;
        case 2: addAdminOrStaff(); break;
        case 3: removeAdminOrStaff(); break;
        case 4: updateAdminOrStaff(); break;
        }
        system("pause");
    } while (choice != 0);
}

/* ================= LIST ADMIN & STAFF ================= */
void listAdminStaff() {
    sql::Connection* conn = connectDB();

    cout << "\n--- ADMIN LIST ---\n";
    auto st1 = conn->createStatement();
    auto rs1 = st1->executeQuery(
        "SELECT AdminId, name, Phone_number, Address, Gender, MaritalStatus, "
        "StartWorkingTime, ManagerId, ActorID FROM admin"
    );

    cout << left
        << setw(8) << "ID"
        << setw(15) << "Name"
        << setw(12) << "Phone"
        << setw(15) << "Address"
        << setw(10) << "Gender"
        << setw(12) << "Status"
        << setw(15) << "StartDate"
        << setw(10) << "Manager"
        << setw(8) << "Actor" << endl;

    cout << string(120, '-') << endl;

    while (rs1->next()) {
        cout << left
            << setw(8) << rs1->getString("AdminId")
            << setw(15) << rs1->getString("name")
            << setw(12) << rs1->getString("Phone_number")
            << setw(15) << rs1->getString("Address")
            << setw(10) << rs1->getString("Gender")
            << setw(12) << rs1->getString("MaritalStatus")
            << setw(15) << rs1->getString("StartWorkingTime")
            << setw(10) << rs1->getString("ManagerId")
            << setw(8) << rs1->getString("ActorID") << endl;
    }

    delete rs1;
    delete st1;

    cout << "\n--- STAFF LIST ---\n";
    auto st2 = conn->createStatement();
    auto rs2 = st2->executeQuery(
        "SELECT StaffID, Name, Phone_number, Address, Gender, MaritalStatus, "
        "StartWorkingTime, ManagerID, ActorID FROM staff"
    );

    cout << left
        << setw(8) << "ID"
        << setw(15) << "Name"
        << setw(12) << "Phone"
        << setw(15) << "Address"
        << setw(10) << "Gender"
        << setw(12) << "Status"
        << setw(15) << "StartDate"
        << setw(10) << "Manager"
        << setw(8) << "Actor" << endl;

    cout << string(120, '-') << endl;

    while (rs2->next()) {
        cout << left
            << setw(8) << rs2->getString("StaffID")
            << setw(15) << rs2->getString("Name")
            << setw(12) << rs2->getString("Phone_number")
            << setw(15) << rs2->getString("Address")
            << setw(10) << rs2->getString("Gender")
            << setw(12) << rs2->getString("MaritalStatus")
            << setw(15) << rs2->getString("StartWorkingTime")
            << setw(10) << rs2->getString("ManagerID")
            << setw(8) << rs2->getString("ActorID") << endl;
    }

    delete rs2;
    delete st2;
    delete conn;
}

/* ================= ADD ADMIN / STAFF ================= */
void addAdminOrStaff() {
    int type;
    cout << "\n1. Add Admin\n2. Add Staff\nChoice: ";
    cin >> type;

    sql::Connection* conn = connectDB();

    try {
        if (type == 1) {
            string name, phone, address, gender, status, startDate, password, managerID;
            string adminID = generateAdminID(conn);
            string actorID = generateActorID(conn, "AT");

            cout << "Name: "; cin >> ws; getline(cin, name);
            cout << "Phone: "; cin >> phone;
            cout << "Address: "; cin >> ws; getline(cin, address);
            cout << "Gender: "; cin >> gender;
            cout << "Marital Status: "; cin >> status;
            cout << "Start Date (YYYY-MM-DD): "; cin >> startDate;
            cout << "Password: "; cin >> password;
            cout << "Manager ID: "; cin >> managerID;

            auto psActor = conn->prepareStatement(
                "INSERT INTO actor (ActorID) VALUES (?)"
            );
            psActor->setString(1, actorID);
            psActor->execute();
            delete psActor;

            auto ps = conn->prepareStatement(
                "INSERT INTO admin "
                "(AdminId, name, Phone_number, Address, Gender, MaritalStatus, "
                "StartWorkingTime, Password, ManagerId, ActorID) "
                "VALUES (?,?,?,?,?,?,?,?,?,?)"
            );

            ps->setString(1, adminID);
            ps->setString(2, name);
            ps->setString(3, phone);
            ps->setString(4, address);
            ps->setString(5, gender);
            ps->setString(6, status);
            ps->setString(7, startDate);
            ps->setString(8, password);
            ps->setString(9, managerID);
            ps->setString(10, actorID);
            ps->execute();

            cout << "Admin added successfully. ID = " << adminID << endl;
            delete ps;
        }
        else if (type == 2) {
            string name, phone, address, gender, status, startDate, password, managerID;
            string staffID = generateStaffID(conn);
            string actorID = generateActorID(conn, "ST");

            cout << "Name: "; cin >> ws; getline(cin, name);
            cout << "Phone: "; cin >> phone;
            cout << "Address: "; cin >> ws; getline(cin, address);
            cout << "Gender: "; cin >> gender;
            cout << "Marital Status: "; cin >> status;
            cout << "Start Date (YYYY-MM-DD): "; cin >> startDate;
            cout << "Password: "; cin >> password;
            cout << "Manager ID: "; cin >> managerID;

            auto psActor = conn->prepareStatement(
                "INSERT INTO actor (ActorID) VALUES (?)"
            );
            psActor->setString(1, actorID);
            psActor->execute();
            delete psActor;

            auto ps = conn->prepareStatement(
                "INSERT INTO staff "
                "(StaffID, Name, Phone_number, Address, Gender, MaritalStatus, "
                "StartWorkingTime, Password, ManagerID, ActorID) "
                "VALUES (?,?,?,?,?,?,?,?,?,?)"
            );

            ps->setString(1, staffID);
            ps->setString(2, name);
            ps->setString(3, phone);
            ps->setString(4, address);
            ps->setString(5, gender);
            ps->setString(6, status);
            ps->setString(7, startDate);
            ps->setString(8, password);
            ps->setString(9, managerID);
            ps->setString(10, actorID);
            ps->execute();

            cout << "Staff added successfully. ID = " << staffID << endl;
            delete ps;
        }
    }
    catch (sql::SQLException& e) {
        cout << "SQL Error: " << e.what() << endl;
    }

    delete conn;
}

/* ================= REMOVE ADMIN / STAFF（改良版） ================= */
void removeAdminOrStaff() {
    int type;
    cout << "\n1. Remove Admin\n2. Remove Staff\nChoice: ";
    cin >> type;

    string id, actorID;
    sql::Connection* conn = connectDB();

    try {
        /* ================= REMOVE ADMIN ================= */
        if (type == 1) {

            /*  DISPLAY ADMIN LIST FIRST */
            cout << "\n=========== ADMIN LIST ===========\n";
            auto st = conn->createStatement();
            auto rsList = st->executeQuery(
                "SELECT AdminId, name FROM admin"
            );

            bool found = false;
            while (rsList->next()) {
                found = true;
                cout << rsList->getString("AdminId")
                    << " - " << rsList->getString("name") << endl;
            }

            delete rsList;
            delete st;

            if (!found) {
                cout << "\nNo admin found.\n";
                delete conn;
                return;
            }

            /*  ASK ADMIN ID */
            cout << "\nEnter Admin ID to remove (0 Back): ";
            cin >> id;
            if (id == "0") {
                delete conn;
                return;
            }

            /*  GET ACTOR ID */
            auto psGet = conn->prepareStatement(
                "SELECT ActorID FROM admin WHERE AdminId=?"
            );
            psGet->setString(1, id);
            auto rs = psGet->executeQuery();

            if (!rs->next()) {
                cout << "\n[ERROR] Admin not found.\n";
                delete rs;
                delete psGet;
                delete conn;
                return;
            }

            actorID = rs->getString("ActorID");
            delete rs;
            delete psGet;

            /*  DELETE ADMIN */
            auto psDelAdmin = conn->prepareStatement(
                "DELETE FROM admin WHERE AdminId=?"
            );
            psDelAdmin->setString(1, id);
            psDelAdmin->execute();
            delete psDelAdmin;

            /*  DELETE ACTOR */
            auto psDelActor = conn->prepareStatement(
                "DELETE FROM actor WHERE ActorID=?"
            );
            psDelActor->setString(1, actorID);
            psDelActor->execute();
            delete psDelActor;

            cout << "\nAdmin removed successfully.\n";
        }

        /* ================= REMOVE STAFF ================= */
        else if (type == 2) {

            /*  DISPLAY STAFF LIST FIRST */
            cout << "\n=========== STAFF LIST ===========\n";
            auto st = conn->createStatement();
            auto rsList = st->executeQuery(
                "SELECT StaffID, Name FROM staff"
            );

            bool found = false;
            while (rsList->next()) {
                found = true;
                cout << rsList->getString("StaffID")
                    << " - " << rsList->getString("Name") << endl;
            }

            delete rsList;
            delete st;

            if (!found) {
                cout << "\nNo staff found.\n";
                delete conn;
                return;
            }

           
            cout << "\nEnter Staff ID to remove (0 Back): ";
            cin >> id;
            if (id == "0") {
                delete conn;
                return;
            }

            
            auto psGet = conn->prepareStatement(
                "SELECT ActorID FROM staff WHERE StaffID=?"
            );
            psGet->setString(1, id);
            auto rs = psGet->executeQuery();

            if (!rs->next()) {
                cout << "\n[ERROR] Staff not found.\n";
                delete rs;
                delete psGet;
                delete conn;
                return;
            }

            actorID = rs->getString("ActorID");
            delete rs;
            delete psGet;

           
            auto psDelStaff = conn->prepareStatement(
                "DELETE FROM staff WHERE StaffID=?"
            );
            psDelStaff->setString(1, id);
            psDelStaff->execute();
            delete psDelStaff;

           
            auto psDelActor = conn->prepareStatement(
                "DELETE FROM actor WHERE ActorID=?"
            );
            psDelActor->setString(1, actorID);
            psDelActor->execute();
            delete psDelActor;

            cout << "\nStaff removed successfully.\n";
        }
    }
    catch (sql::SQLException& e) {
        cout << "\n[SQL ERROR] " << e.what() << endl;
    }

    delete conn;
}

void updateAdminOrStaff() {
    int type;
    cout << "\n1. Update Admin\n2. Update Staff\nChoice: ";
    cin >> type;

    string table, idCol, nameCol;
    if (type == 1) {
        table = "admin";
        idCol = "AdminId";
        nameCol = "name";
    }
    else if (type == 2) {
        table = "staff";
        idCol = "StaffID";
        nameCol = "Name";
    }
    else return;

    sql::Connection* conn = connectDB();

    try {
        /* ================= DISPLAY LIST ================= */
        cout << "\n=========== LIST ===========\n";
        auto st = conn->createStatement();
        auto rsList = st->executeQuery(
            "SELECT " + idCol + ", " + nameCol + " FROM " + table
        );

        bool found = false;
        while (rsList->next()) {
            found = true;
            cout << rsList->getString(idCol)
                << " - " << rsList->getString(nameCol) << endl;
        }

        delete rsList;
        delete st;

        if (!found) {
            cout << "\nNo record found.\n";
            delete conn;
            return;
        }

        /* ================= SELECT ID ================= */
        string id;
        cout << "\nEnter ID to update (0 Back): ";
        cin >> id;
        if (id == "0") {
            delete conn;
            return;
        }

        /* ================= FIELD SELECTION ================= */
        int field;
        cout << "\n--- Select Field to Modify ---\n";
        cout << "1. Name\n";
        cout << "2. Phone Number\n";
        cout << "3. Address\n";
        cout << "4. Gender\n";
        cout << "5. Marital Status\n";
        cout << "6. Password\n";
        cout << "0. Cancel\n";
        cout << "Choice: ";
        cin >> field;

        if (field == 0) {
            delete conn;
            return;
        }

        string column;
        switch (field) {
        case 1: column = nameCol; break;
        case 2: column = "Phone_number"; break;
        case 3: column = "Address"; break;
        case 4: column = "Gender"; break;
        case 5: column = "MaritalStatus"; break;
        case 6: column = "Password"; break;
        default:
            cout << "\nInvalid choice.\n";
            delete conn;
            return;
        }

        /* ================= INPUT NEW VALUE ================= */
        string newValue;
        cout << "Enter new value: ";
        cin >> ws;
        getline(cin, newValue);

        /* ================= UPDATE ================= */
        string query =
            "UPDATE " + table +
            " SET " + column + "=? WHERE " + idCol + "=?";

        auto ps = conn->prepareStatement(query);
        ps->setString(1, newValue);
        ps->setString(2, id);
        ps->execute();

        delete ps;
        cout << "\nRecord updated successfully.\n";
    }
    catch (sql::SQLException& e) {
        cout << "\n[SQL ERROR] " << e.what() << endl;
    }

    delete conn;
}
