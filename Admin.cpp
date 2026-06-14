#include "Admin.h"

#include <iostream>
using namespace std;

// Menus
#include "InventoryMenu.h"
#include "SalesMenu.h"
#include "PurchaseMenu.h"
#include "SupplierMenu.h"
#include "ManageInventory.h"

void adminMenu(string adminID) {
    int choice;

    do {
        system("cls");//clear screen before display menu

        cout << "=====================================\n";
        cout << "            ADMIN MAIN MENU           \n";
        cout << "=====================================\n";
        cout << "Logged in as Admin: " << adminID << "\n";
        cout << "-------------------------------------\n";
        cout << "1. Inventory Menu\n";
        cout << "2. Sales Menu\n";
        cout << "3. Purchase Menu\n";
        cout << "4. Supplier Menu\n";
        cout << "5. Manage Inventory\n";
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
            inventoryMenu();
            break;

        case 2:
            salesMenu();
            break;

        case 3:
            purchaseMenu();
            break;

        case 4:
            supplierMenu();
            break;

        case 5:
            manageInventoryMenu();
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
