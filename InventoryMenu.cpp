#include "InventoryMenu.h"
#include <iostream>


#include "Inventory.h"
#include "InputHelper.h"

using namespace std;

void inventoryMenu() {
    int choice;

    while (true) {
        system("cls");

        cout << "=====================================\n";
        cout << "         INVENTORY MENU\n";
        cout << "=====================================\n";
        cout << "1. Insert Inventory\n";
        cout << "2. Update Inventory\n";
        cout << "3. Remove Inventory\n";
        cout << "4. View Inventory\n";
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
            insertInventory();
            break;

        case 2:
            updateInventory();
            break;

        case 3:
            deleteInventory();   
            break;

        case 4:
            viewInventory();
            break;

        case 0:
            return;

        default:
            cout << "[ERROR] Invalid choice.\n";
            pauseScreen();
        }
    }
}
