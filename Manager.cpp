#include "Manager.h"
#include <iostream>


#include "InventoryMenu.h"
#include "SalesMenu.h"
#include "PurchaseMenu.h"
#include "SupplierMenu.h"
#include "Set_Position_Menu.h"
#include "ManageInventory.h"


#include "Analytics.h"

using namespace std;

void managerMenu(string managerID) {
    int choice;

    while (true) {
        system("cls");

        cout << "============================================\n";
        cout << "          MANAGER DASHBOARD\n";
        cout << "============================================\n";
        cout << "Logged in as : Manager [" << managerID << "]\n";
        cout << "--------------------------------------------\n";
        cout << "1. Inventory Menu\n";
        cout << "2. Sales Menu\n";
        cout << "3. Purchase / Invoice Menu\n";
        cout << "4. Supplier Menu\n";
        cout << "5. Set Position\n";
        cout << "6. Manage Inventory Menu\n";
        cout << "7. Analytics & Reports\n";   
        cout << "0. Logout\n";
        cout << "--------------------------------------------\n";
        cout << "Choice: ";

        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        system("cls");

        switch (choice) {
        case 1: inventoryMenu(); break;
        case 2: salesMenu(); break;
        case 3: purchaseMenu(); break;
        case 4: supplierMenu(); break;
        case 5: setPositionMenu(); break;
        case 6: manageInventoryMenu(); break;

        case 7: analyticsMenu(); break;   

        case 0:
            cout << "Logging out...\n";
            system("pause");
            return;

        default:
            cout << "[ERROR] Invalid option.\n";
            system("pause");
        }
    }
}
