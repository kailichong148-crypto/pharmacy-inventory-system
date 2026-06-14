#include "Auth.h"
#include "Database.h"
#include "Session.h"

#include "Manager.h"
#include "Admin.h"
#include "Staff.h"
#include "InputHelper.h"
#include <iostream>
#include <conio.h> 
#include <thread>
#include <chrono>
#include <ctime>

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;
void printTitle() {
    cout <<
        "  ____  _   _    _    ____  __  __    _    \n"
        " |  _ \\| | | |  / \\  |  _ \\|  \\/  |  / \\   \n"
        " | |_) | |_| | / _ \\ | |_) | |\\/| | / _ \\  \n"
        " |  __/|  _  |/ ___ \\|  _ <| |  | |/ ___ \\ \n"
        " |_|   |_| |_/_/   \\_\\_| \\_\\_|  |_/_/   \\_\\\n"
        "------------------------------------------------\n"
        "                 PHARMA SYSTEM                  \n"
        "------------------------------------------------\n";
}




/* ================= HEADER WITH DATE + ROLE ================= */
void printHeader(const string& role) {
    time_t now = time(nullptr);
    char buf[26];
    ctime_s(buf, sizeof(buf), &now);

    printTitle();

    cout << "System Date : " << buf;
    if (!role.empty()) {
        cout << "Login Role  : " << role << endl;
    }
    cout << "------------------------------------------------\n\n";
}



/* ================= WELCOME SCREEN ================= */
void showWelcomeScreen() {
    system("cls");
    printHeader();

    cout << "|  Welcome to Pharmacy Inventory Management System          |\n";
    cout << "|  Your patience during this process is greatly appreciated |\n";
    cout << "|                                                           |\n";
    cout << "|  1. Login                                                 |\n";
    cout << "|  2. Exit                                                  |\n";
    cout << "============================================================\n";

    cout << "\nEnter the option: ";
}

/* ================= LOGIN HEADER ================= */
void showLoginHeader(const string& role) {
    system("cls");
    printHeader(role);
    cout << "--- LOGIN AS " << role << " ---\n\n";
}

/* ================= LOGIN SUCCESS ================= */
void showLoginSuccess(const string& role, const string& id) {
    system("cls");
    printHeader(role);

    cout << "Login Successful!\n";
    cout << "Welcome " << role << " [" << id << "]\n\n";
    
   pauseScreen();
}

/* ================= LOGIN FAIL ================= */
void showLoginFail(int attempts) {
    system("cls");
    printTitle();
    cout << "\nInvalid ID or Password\n";
    cout << "Attempt " << attempts << " / 3\n\n";
    cout << "Press any key to try again...";
     pauseScreen();
}

/* ================= HIDDEN PASSWORD ================= */
string getHiddenPassword() {
    string password;
    char ch;

    while (true) {
        ch = _getch();
        if (ch == 13) break;          // Enter
        if (ch == 8 && !password.empty()) {
            password.pop_back();
            cout << "\b \b";
        }
        else if (ch != 8) {
            password.push_back(ch);
            cout << "*";
        }
    }
    cout << endl;
    return password;
}

/* ================= CHOOSE ROLE ================= */
int chooseRole() {
    int role;

    system("cls");
    printTitle();
    cout << "\n=== LOGIN ROLE ===\n";
    cout << "1. Manager\n";
    cout << "2. Admin\n";
    cout << "3. Staff\n";
    cout << "Choice: ";
    cin >> role;

    return role;
}

/* ================= LOGIN MAIN ================= */
void login() {
    int attempts = 0;

    while (true) {
        showWelcomeScreen();

        int option;
        cin >> option;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (option == 2) return;
        if (option != 1) continue;

        int role = chooseRole();
        string id, password, query, roleName;

        sql::Connection* conn = connectDB();

        try {
            if (role == 1) {
                roleName = "Manager";
                showLoginHeader(roleName);
                cout << "Manager ID: ";
                cin >> id;
                cout << "Password: ";
                password = getHiddenPassword();
                query = "SELECT ActorID FROM manager WHERE ManagerID=? AND Password=?";
            }
            else if (role == 2) {
                roleName = "Admin";
                showLoginHeader(roleName);
                cout << "Admin ID: ";
                cin >> id;
                cout << "Password: ";
                password = getHiddenPassword();
                query = "SELECT ActorID FROM admin WHERE AdminId=? AND Password=?";
            }
            else if (role == 3) {
                roleName = "Staff";
                showLoginHeader(roleName);
                cout << "Staff ID: ";
                cin >> id;
                cout << "Password: ";
                password = getHiddenPassword();
                query = "SELECT ActorID FROM staff WHERE StaffID=? AND Password=?";
            }
            else {
                delete conn;
                continue;
            }

            auto ps = conn->prepareStatement(query);
            ps->setString(1, id);
            ps->setString(2, password);
            auto rs = ps->executeQuery();

            if (rs->next()) {
                currentUserID = id;
                currentActorID = rs->getString("ActorID");

                showLoginSuccess(roleName, id);

                delete rs;
                delete ps;
                delete conn;

                if (role == 1) managerMenu(id);
                else if (role == 2) adminMenu(id);
                else staffMenu(id);

                return;
            }

            attempts++;
            showLoginFail(attempts);

            delete rs;
            delete ps;
            delete conn;

            if (attempts >= 3) {
                system("cls");
                printTitle();
                cout << "\nToo many failed attempts.\n";
                cout << "Login locked for 1 minute...\n";
                this_thread::sleep_for(chrono::minutes(1));
                attempts = 0;
            }
        }
        catch (...) {
            system("cls");
            cout << "[System Error]\n";
            if (conn) delete conn;
        }
    }
}

