#ifndef PURCHASE_H
#define PURCHASE_H

#include <string>
#include <cppconn/connection.h>

using namespace std;

string generateOrderID(sql::Connection* conn);

void displaySuppliers();
void displayInventoryByCategory(const string& category);

void insertPurchase();
void updatePurchase();
void cancelPurchase();
void viewPurchaseReport();

#endif
