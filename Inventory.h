#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <cppconn/connection.h>
#include <iostream>

using namespace std;

/* ===== ID / Code / Batch Generators ===== */
string generateInventoryID(sql::Connection* conn);
string generateBatchNumber(sql::Connection* conn, const std::string& category);
string generateItemCode(sql::Connection* conn, const string& category, const string& productName);

/* ===== Category ===== */
std::string chooseCategory();

/* ===== Inventory CRUD ===== */
void viewInventory();
void insertInventory();
void updateInventory();
void deleteInventory();
void displayInventoryList();
#endif

