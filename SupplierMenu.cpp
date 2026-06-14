#include "SupplierMenu.h"
#include "Supplier.h"
#include "InputHelper.h"

#include <iostream>
using namespace std;

/* ================= SUPPLIER MENU ================= */
void supplierMenu() {
    int choice;

    do {
        system("cls");

        cout << "=====================================\n";
        cout << "        SUPPLIER MANAGEMENT MENU      \n";
        cout << "=====================================\n";
        cout << "1. Add Supplier\n";
        cout << "2. Update Supplier\n";
        cout << "3. Remove Supplier\n";
        cout << "4. View Supplier List\n";
        cout << "0. Back\n";
        cout << "-------------------------------------\n";
        cout << "Choice: ";

        if (!inputInt(choice)) {
            system("cls");
            return;
        }

        switch (choice) {
        case 1:
            insertSupplier();
            break;

        case 2:
            updateSupplier();
            break;

        case 3:
            deleteSupplier();   
            break;

        case 4:
            system("cls");
            viewSupplier();
            break;

        case 0:
            system("cls");
            return;

        default:
            cout << "\n[ERROR] Invalid option.\n";
            cout << "Press Enter to continue...";
            cin.get();
        }

    } while (true);
}
