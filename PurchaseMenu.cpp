#include "PurchaseMenu.h"
#include "Purchase.h"
#include "InputHelper.h"

#include <iostream>
using namespace std;

/* ================= PURCHASE MENU ================= */
void purchaseMenu() {
    int choice;

    do {
        system("cls");

        cout << "=====================================\n";
        cout << "        PURCHASE MANAGEMENT MENU      \n";
        cout << "=====================================\n";
        cout << "1. Add Purchase (Create Order)\n";
        cout << "2. Update Purchase (Receive / Pending)\n";
        cout << "3. Cancel Purchase\n";
        cout << "4. View Purchase Report\n";
        cout << "0. Back\n";
        cout << "-------------------------------------\n";
        cout << "Choice: ";

        if (!inputInt(choice)) {
            system("cls");
            return;
        }

        switch (choice) {
        case 1:
            insertPurchase();
            break;

        case 2:
            updatePurchase();
            break;

        case 3:
            cancelPurchase();   
            break;

        case 4:
            viewPurchaseReport();
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
