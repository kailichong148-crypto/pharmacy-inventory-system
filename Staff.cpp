#include "Staff.h"

#include <iostream>
using namespace std;


#include "SalesMenu.h"

void staffMenu(string staffID) {
    int choice;

    do {
        system("cls");

        cout << "=====================================\n";
        cout << "            STAFF MAIN MENU           \n";
        cout << "=====================================\n";
        cout << "Logged in as Staff: " << staffID << "\n";
        cout << "-------------------------------------\n";
        cout << "1. Sales Menu\n";
        cout << "0. Logout\n";
        cout << "-------------------------------------\n";
        cout << "Choice: ";

        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
        case 1:
            salesMenu();
            break;

        case 0:
            system("cls");
            cout << "Logging out...\n";
            return;

        default:
            cout << "\n[ERROR] Invalid option.\n";
            cout << "Press Enter to continue...";
            cin.ignore(10000, '\n');
            cin.get();
        }

    } while (true);
}
