#include "Analytics.h"
#include "Database.h"

#include <iostream>
#include <iomanip>
#include "InputHelper.h"

#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

using namespace std;

/* ================= ANALYTICS MENU ================= */
void analyticsMenu() {
    int choice;

    do {
        system("cls");
        cout << "=========== ANALYTICS & REPORTS ===========\n";
        cout << "1. Most Popular Product by Category\n";
        cout << "2. Monthly Sales Trend\n";
        cout << "0. Back\n";
        cout << "-------------------------------------------\n";
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
            viewPopularProductByCategory();
            
            break;

        case 2:
            viewMonthlySalesTrendAnalytics();
           
            break;

        case 0:
            return;

        default:
            cout << "Invalid choice.\n";
            system("pause");
        }
    } while (true);
}


void viewPopularProductByCategory() {
    system("cls");

    int choice;
    string category;

    cout << "=== MOST POPULAR PRODUCT BY CATEGORY ===\n";
    cout << "1. Personal Care\n";
    cout << "2. Household Essentials\n";
    cout << "3. Health & Beauty\n";
    cout << "4. Toiletries\n";
    cout << "Choice: ";
    cin >> choice;

    switch (choice) {
    case 1: category = "Personal Care"; break;
    case 2: category = "Household Essentials"; break;
    case 3: category = "Health & Beauty"; break;
    case 4: category = "Toiletries"; break;
    default:
        cout << "Invalid choice.\n";
        system("pause");
        return;
    }

    sql::Connection* conn = connectDB();

    auto ps = conn->prepareStatement(
        "SELECT p.ProductName, SUM(d.Quantity) AS TotalSold "
        "FROM sales_detail d "
        "JOIN product p ON d.ItemCode = p.ItemCode "
        "WHERE p.Category = ? "
        "GROUP BY p.ProductName "
        "ORDER BY TotalSold DESC"
    );

    ps->setString(1, category);
    auto rs = ps->executeQuery();

    // ---------- reed data,use the higher sales  ----------
    const int MAX_BAR_WIDTH = 40;
    int maxSold = 0;

    struct Row {
        string name;
        int sold;
    };

    Row rows[50];   
    int count = 0;

    while (rs->next()) {
        rows[count].name = rs->getString("ProductName");
        rows[count].sold = rs->getInt("TotalSold");

        if (rows[count].sold > maxSold)
            maxSold = rows[count].sold;

        count++;
    }

    if (count == 0) {
        cout << "\nNo sales data found for this category.\n";
        delete rs;
        delete ps;
        delete conn;
        system("pause");
        return;
    }

    cout << "\nCategory: " << category << endl;
    cout << string(75, '-') << endl;
    cout << left
        << setw(5) << "No"
        << setw(25) << "Product Name"
        << setw(12) << "Total Sold"
        << "Sales Bar\n";
    cout << string(75, '-') << endl;

    // ----------  ----------
    for (int i = 0; i < count; i++) {
        int barLength = (rows[i].sold * MAX_BAR_WIDTH) / maxSold;
        if (barLength < 1) barLength = 1;

        cout << left
            << setw(5) << (i + 1)
            << setw(25) << rows[i].name
            << setw(12) << rows[i].sold;

        for (int j = 0; j < barLength; j++)
            cout << char(219); 

        cout << endl;
    }

    delete rs;
    delete ps;
    delete conn;

    pauseScreen();
}




void viewMonthlySalesTrendAnalytics() {
    system("cls");
    cout << "\n=== MONTHLY SALES TREND ===\n\n";

    sql::Connection* conn = connectDB();

    try {
        auto ps = conn->prepareStatement(
            "SELECT YEAR(SaleDate) AS Year, "
            "MONTH(SaleDate) AS Month, "
            "SUM(GrandTotal) AS MonthlyTotal "
            "FROM sales "
            "GROUP BY YEAR(SaleDate), MONTH(SaleDate) "
            "ORDER BY YEAR(SaleDate), MONTH(SaleDate)"
        );

        auto rs = ps->executeQuery();

        static const string monthNames[] = {
            "", "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };

        cout << left
            << setw(12) << "Month"
            << setw(8) << "Year"
            << setw(15) << "Sales (RM)"
            << setw(15) << "Change (RM)"
            << setw(15) << "Change (%)"
            << setw(15) << "Trend"
            << endl;

        cout << string(80, '-') << endl;

        double previous = -1;
        bool hasData = false;

        while (rs->next()) {
            hasData = true;

            int year = rs->getInt("Year");
            int month = rs->getInt("Month");
            double current = rs->getDouble("MonthlyTotal");

            cout << left
                << setw(12) << monthNames[month]
                << setw(8) << year
                << setw(15) << fixed << setprecision(2) << current;

            if (previous < 0) {
                cout << setw(15) << "-"
                    << setw(15) << "-"
                    << setw(15) << "First record";
            }
            else {
                double diff = current - previous;
                double percent = (diff / previous) * 100.0;

                cout << setw(15) << fixed << setprecision(2) << diff
                    << setw(15) << fixed << setprecision(2) << percent;

                if (diff > 0)
                    cout << setw(15) << "Increase";
                else if (diff < 0)
                    cout << setw(15) << "Decrease";
                else
                    cout << setw(15) << "No Change";
            }

            cout << endl;
            previous = current;
        }

        if (!hasData) {
            cout << "No sales records found.\n";
        }

        delete rs;
        delete ps;
    }
    catch (sql::SQLException& e) {
        cout << "\n[SQL ERROR] " << e.what() << endl;
    }

    delete conn;
    pauseScreen();
}
