#include "Login.h"
#include <iostream>
#include <conio.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

string getHiddenPassword() {
    string password;
    char ch;
    while ((ch = _getch()) != 13) {
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

int chooseRole() {
    int role;
    cout << "1. Manager\n2. Admin\n3. Staff\nChoice: ";
    cin >> role;
    return role;
}

string login(sql::Connection* conn, int role) {
    string id, password, query, label;

    if (role == 1) { label = "ManagerID"; query = "SELECT * FROM manager WHERE ManagerID=? AND Password=?"; }
    else if (role == 2) { label = "AdminID"; query = "SELECT * FROM admin WHERE AdminID=? AND Password=?"; }
    else if (role == 3) { label = "StaffID"; query = "SELECT * FROM staff WHERE StaffID=? AND Password=?"; }
    else return "";

    cout << label << ": ";
    cin >> id;
    cout << "Password: ";
    password = getHiddenPassword();

    auto ps = conn->prepareStatement(query);
    ps->setString(1, id);
    ps->setString(2, password);

    auto rs = ps->executeQuery();
    bool ok = rs->next();

    delete rs;
    delete ps;

    if (ok) {
        cout << "Login successful\n";
        return id;
    }
    cout << "Invalid login\n";
    return "";
}
