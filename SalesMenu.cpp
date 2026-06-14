#include "SalesMenu.h"
#include <iostream>

#include "Sales.h"   

using namespace std;

void salesMenu() {
    int choice;

    while (true) {
        system("cls");

        cout << "=====================================\n";
        cout << "              SALES MENU\n";
        cout << "=====================================\n";
        cout << "1. Insert Sales\n";
        cout << "2. Update Sales\n";
        cout << "3. Customer Refund\n";
        cout << "4. View Sales\n";
        cout << "0. Back\n";
        cout << "-------------------------------------\n";
        cout << "Choice: ";

        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        system("cls");

        switch (choice) {
        case 1:
            insertSales();
            break;

        case 2:
            updateSales();
            break;

        case 3:
            refundSales();      
            break;

        case 4:
            viewSalesMenu();
            break;

        case 0:
            return;

        default:
            cout << "[ERROR] Invalid option.\n";
            cout << "Press Enter to continue...";
            cin.ignore(10000, '\n');
            cin.get();
        }
    }
}
