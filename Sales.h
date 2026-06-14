#ifndef SALES_H
#define SALES_H

#include <string>
#include <cppconn/connection.h>

using namespace std;

/* ================= ID GENERATOR ================= */
string generateSalesID(sql::Connection* conn);

/* ================= MENU ================= */
void viewSalesMenu();

/* ================= SALES CRUD ================= */
void insertSales();
void viewSales();
bool viewSalesByMonth(int month, int year);
void viewSalesByYear(int year);
void viewDailySales(const string& date);
void updateSales();
void refundSales();

/* ================= HELPER DISPLAY ================= */
void displayInventoryForSales();

#endif
