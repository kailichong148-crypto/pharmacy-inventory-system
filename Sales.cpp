#include "Sales.h"
#include "Database.h"
#include "Session.h"
#include "promotion.h"
#include "InputHelper.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

/* ================= GENERATE SALES ID ================= */
string generateSalesID(sql::Connection* conn) {
    auto rs = conn->createStatement()->executeQuery(
        "SELECT SaleID FROM sales ORDER BY SaleID DESC LIMIT 1"
    );

    int num = 1;
    if (rs->next())
        num = stoi(rs->getString("SaleID").substr(1)) + 1;

    delete rs;

    stringstream ss;
    ss << "S" << setw(3) << setfill('0') << num;
    return ss.str();
}

/* ================= INVENTORY LIST ================= */
void displayInventoryForSales() {
    sql::Connection* conn = connectDB();

    auto rs = conn->createStatement()->executeQuery(
        "SELECT p.Category, i.ItemCode, p.ProductName, "
        "SUM(i.quantity_in_stock) AS Stock "
        "FROM inventory i "
        "JOIN product p ON i.ItemCode = p.ItemCode "
        "WHERE i.quantity_in_stock > 0 "
        "GROUP BY p.Category, i.ItemCode, p.ProductName "
        "ORDER BY p.Category, i.ItemCode"
    );

    string currentCategory = "";

    while (rs->next()) {
        string category = rs->getString("Category");

        // 当检测到新分类时，显示分类标题与表头
        if (category != currentCategory) {
            currentCategory = category;

            cout << "[" << currentCategory << "]\n";
            cout << left
                << setw(10) << "Item"
                << setw(30) << "Product"
                << setw(8) << "Stock" << endl;
        }

        cout << left
            << setw(10) << rs->getString("ItemCode")
            << setw(30) << rs->getString("ProductName")
            << setw(8) << rs->getInt("Stock") << endl;
    }

    delete rs;
    delete conn;
}

/* ================= VIEW SALES MENU ================= */
void viewSalesMenu() {
    int choice;

    while (true) {
        system("cls");

        cout << "===== VIEW SALES =====\n";
        cout << "1. View Sales by Day\n";
        cout << "2. View Sales by Month\n";
        cout << "3. View Sales by Year\n";
        cout << "0. Back\n";
        cout << "Choice: ";

        if (!inputInt(choice)) return;

        switch (choice) {

        case 1: {
            string date;
            cout << "Enter Date (YYYY-MM-DD, 0 Back): ";
            cin >> date;

            if (date == "0") break;

            viewDailySales(date);  
            break;
        }

        case 2: {
            int month, year;
            cout << "Month (1-12, 0 Back): ";
            if (!inputInt(month) || month == 0) break;

            cout << "Year: ";
            if (!inputInt(year)) break;

            viewSalesByMonth(month, year);
            break;
        }

        case 3: {
            int year;
            cout << "Year (0 Back): ";
            if (!inputInt(year) || year == 0) break;

            viewSalesByYear(year);
            break;
        }

        case 0:
            return;

        default:
            cout << "[ERROR] Invalid choice.\n";
            pauseScreen();
        }
    }
}



/* ================= VIEW ALL SALES ================= */
void viewSales() {
    system("cls");
    sql::Connection* conn = connectDB();

    auto rs = conn->createStatement()->executeQuery(
        "SELECT s.*, d.* FROM sales s "
        "JOIN sales_detail d ON s.SaleID=d.SaleID "
        "ORDER BY s.SaleDate DESC, d.SaleDetailID"
    );

    string cur = "";
    string lastSaleID = "";         
    double totalSales = 0.0;        

    while (rs->next()) {
        if (rs->getString("SaleID") != cur) {
            cur = rs->getString("SaleID");

            // ✅ 只在新 SaleID 时加一次
            if (cur != lastSaleID) {
                totalSales += rs->getDouble("GrandTotal");
                lastSaleID = cur;
            }

            cout << "\n==============================\n";
            cout << "Sale ID : " << cur << endl;
            cout << "Date    : " << rs->getString("SaleDate") << endl;
            cout << "Payment : " << rs->getString("PaymentMethod") << endl;
            cout << "Total   : RM " << fixed << setprecision(2)
                << rs->getDouble("GrandTotal") << endl;
            cout << "------------------------------\n";
            cout << left << setw(10) << "Item"
                << setw(6) << "Qty"
                << setw(10) << "Price"
                << setw(10) << "Total\n";
        }

        cout << left
            << setw(10) << rs->getString("ItemCode")
            << setw(6) << rs->getInt("Quantity")
            << setw(10) << fixed << setprecision(2) << rs->getDouble("UnitPrice")
            << setw(10) << fixed << setprecision(2) << rs->getDouble("LineTotal")
            << endl;
    }

    // ================= SUMMARY =================
    cout << "\n================================\n";
    cout << "TOTAL SALES : RM "
        << fixed << setprecision(2)
        << totalSales << endl;
    cout << "================================\n";

    delete rs;
    delete conn;
    pauseScreen();
}

bool viewSalesByMonth(int month, int year) {
    system("cls");

    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT s.SaleID, s.SaleDate, s.PaymentMethod, s.GrandTotal, "
        "d.ItemCode, d.Quantity, d.UnitPrice, d.LineTotal "
        "FROM sales s "
        "JOIN sales_detail d ON s.SaleID = d.SaleID "
        "WHERE MONTH(s.SaleDate)=? AND YEAR(s.SaleDate)=? "
        "ORDER BY s.SaleDate DESC, d.SaleDetailID"
    );
    ps->setInt(1, month);
    ps->setInt(2, year);

    auto rs = ps->executeQuery();

    string currentSaleID = "";
    string lastSaleID = "";          
    double totalMonthlySales = 0.0;  
    bool found = false;

    cout << "\n=========== SALES " << month << "/" << year << " ===========\n";

    while (rs->next()) {
        found = true;

        if (rs->getString("SaleID") != currentSaleID) {
            currentSaleID = rs->getString("SaleID");

            if (currentSaleID != lastSaleID) {
                totalMonthlySales += rs->getDouble("GrandTotal");
                lastSaleID = currentSaleID;
            }

            cout << "\n==============================\n";
            cout << "Sale ID : " << currentSaleID << endl;
            cout << "Date    : " << rs->getString("SaleDate") << endl;
            cout << "Payment : " << rs->getString("PaymentMethod") << endl;
            cout << "Total   : RM "
                << fixed << setprecision(2)
                << rs->getDouble("GrandTotal") << endl;
            cout << "------------------------------\n";
            cout << left
                << setw(10) << "Item"
                << setw(6) << "Qty"
                << setw(10) << "Price"
                << setw(10) << "Total" << endl;
        }

        cout << left
            << setw(10) << rs->getString("ItemCode")
            << setw(6) << rs->getInt("Quantity")
            << setw(10) << fixed << setprecision(2)
            << rs->getDouble("UnitPrice")
            << setw(10) << rs->getDouble("LineTotal")
            << endl;
    }

    if (!found)
        cout << "\nNo sales found.\n";
    else {
        cout << "\n================================\n";
        cout << "TOTAL MONTHLY SALES : RM "
            << fixed << setprecision(2)
            << totalMonthlySales << endl;
        cout << "================================\n";
    }

    delete rs;
    delete ps;
    delete conn;

    pauseScreen();
    return found;
}


void viewSalesByYear(int year) {
    system("cls");

    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT s.SaleID, s.SaleDate, s.PaymentMethod, s.GrandTotal, "
        "d.ItemCode, d.Quantity, d.UnitPrice, d.LineTotal "
        "FROM sales s "
        "JOIN sales_detail d ON s.SaleID = d.SaleID "
        "WHERE YEAR(s.SaleDate)=? "
        "ORDER BY s.SaleDate DESC, d.SaleDetailID"
    );
    ps->setInt(1, year);

    auto rs = ps->executeQuery();

    string currentSaleID = "";
    string lastSaleID = "";         
    double totalYearlySales = 0.0;  
    bool found = false;

    cout << "\n=========== SALES YEAR " << year << " ===========\n";

    while (rs->next()) {
        found = true;

        if (rs->getString("SaleID") != currentSaleID) {
            currentSaleID = rs->getString("SaleID");

            if (currentSaleID != lastSaleID) {
                totalYearlySales += rs->getDouble("GrandTotal");
                lastSaleID = currentSaleID;
            }

            cout << "\n==============================\n";
            cout << "Sale ID : " << currentSaleID << endl;
            cout << "Date    : " << rs->getString("SaleDate") << endl;
            cout << "Payment : " << rs->getString("PaymentMethod") << endl;
            cout << "Total   : RM "
                << fixed << setprecision(2)
                << rs->getDouble("GrandTotal") << endl;
            cout << "------------------------------\n";
            cout << left
                << setw(10) << "Item"
                << setw(6) << "Qty"
                << setw(10) << "Price"
                << setw(10) << "Total" << endl;
        }

        cout << left
            << setw(10) << rs->getString("ItemCode")
            << setw(6) << rs->getInt("Quantity")
            << setw(10) << fixed << setprecision(2)
            << rs->getDouble("UnitPrice")
            << setw(10) << rs->getDouble("LineTotal")
            << endl;
    }

    if (!found)
        cout << "\nNo sales found.\n";
    else {
        cout << "\n================================\n";
        cout << "TOTAL YEARLY SALES : RM "
            << fixed << setprecision(2)
            << totalYearlySales << endl;
        cout << "================================\n";
    }

    delete rs;
    delete ps;
    delete conn;

    pauseScreen();
}

void viewDailySales(const string& date) {
    system("cls");

    sql::Connection* conn = connectDB();
    auto ps = conn->prepareStatement(
        "SELECT s.SaleID, s.SaleDate, s.PaymentMethod, s.GrandTotal, "
        "d.ItemCode, d.Quantity, d.UnitPrice, d.LineTotal "
        "FROM sales s "
        "JOIN sales_detail d ON s.SaleID = d.SaleID "
        "WHERE DATE(s.SaleDate)=? "
        "ORDER BY s.SaleDate DESC, d.SaleDetailID"
    );
    ps->setString(1, date);

    auto rs = ps->executeQuery();

    string currentSaleID = "";
    string lastSaleID = "";
    double totalDailySales = 0.0;
    bool found = false;

    cout << "\n=========== SALES DATE " << date << " ===========\n";

    while (rs->next()) {
        found = true;

        if (rs->getString("SaleID") != currentSaleID) {
            currentSaleID = rs->getString("SaleID");

            if (currentSaleID != lastSaleID) {
                totalDailySales += rs->getDouble("GrandTotal");
                lastSaleID = currentSaleID;
            }

            cout << "\n==============================\n";
            cout << "Sale ID : " << currentSaleID << endl;
            cout << "Date    : " << rs->getString("SaleDate") << endl;
            cout << "Payment : " << rs->getString("PaymentMethod") << endl;
            cout << "Total   : RM "
                << fixed << setprecision(2)
                << rs->getDouble("GrandTotal") << endl;
            cout << "------------------------------\n";
            cout << left
                << setw(10) << "Item"
                << setw(6) << "Qty"
                << setw(10) << "Price"
                << setw(10) << "Total" << endl;
        }

        cout << left
            << setw(10) << rs->getString("ItemCode")
            << setw(6) << rs->getInt("Quantity")
            << setw(10) << fixed << setprecision(2)
            << rs->getDouble("UnitPrice")
            << setw(10) << rs->getDouble("LineTotal")
            << endl;
    }

    if (!found)
        cout << "\nNo sales found.\n";
    else {
        cout << "\n================================\n";
        cout << "TOTAL DAILY SALES : RM "
            << fixed << setprecision(2)
            << totalDailySales << endl;
        cout << "================================\n";
    }

    delete rs;
    delete ps;
    delete conn;

    pauseScreen();
}


/* ================= REFUND SALES (USER FRIENDLY + STOCK ROLLBACK) ================= */
void refundSales() {
    system("cls");

    int month, year;

    cout << "===== REFUND SALES =====\n";
    cout << "Month (0 Back): ";
    if (!inputInt(month) || month == 0) return;

    cout << "Year  (0 Back): ";
    if (!inputInt(year) || year == 0) return;

    /* ===== SHOW SALES FIRST (NO PAUSE INSIDE) ===== */
    if (!viewSalesByMonth(month, year)) {
        pauseScreen();
        return;
    }

    /* ===== ASK SALE ID ===== */
    cout << "\nEnter SaleID to refund (0 Back): ";
    string saleID;
    cin >> saleID;

    if (saleID == "0") return;

    sql::Connection* conn = connectDB();

    try {
        /* ===== CHECK SALE EXISTS ===== */
        auto chk = conn->prepareStatement(
            "SELECT 1 FROM sales WHERE SaleID=?"
        );
        chk->setString(1, saleID);
        auto rsChk = chk->executeQuery();

        if (!rsChk->next()) {
            cout << "\n[ERROR] SaleID not found.\n";
            delete rsChk;
            delete chk;
            delete conn;
            pauseScreen();
            return;
        }
        delete rsChk;
        delete chk;

        /* ===== ROLLBACK INVENTORY (FIFO SAFE) ===== */
        auto rsDetail = conn->prepareStatement(
            "SELECT ItemCode, Quantity FROM sales_detail WHERE SaleID=?"
        );
        rsDetail->setString(1, saleID);
        auto rs = rsDetail->executeQuery();

        while (rs->next()) {
            string itemCode = rs->getString("ItemCode");
            int qty = rs->getInt("Quantity");

            auto upd = conn->prepareStatement(
                "UPDATE inventory "
                "SET quantity_in_stock = quantity_in_stock + ? "
                "WHERE ItemCode=? "
                "ORDER BY InventoryID ASC "
                "LIMIT 1"
            );
            upd->setInt(1, qty);
            upd->setString(2, itemCode);
            upd->execute();
            delete upd;
        }

        delete rs;
        delete rsDetail;

        /* ===== DELETE SALES DETAIL ===== */
        auto delDetail = conn->prepareStatement(
            "DELETE FROM sales_detail WHERE SaleID=?"
        );
        delDetail->setString(1, saleID);
        delDetail->execute();
        delete delDetail;

        /* ===== DELETE SALES HEADER ===== */
        auto delHeader = conn->prepareStatement(
            "DELETE FROM sales WHERE SaleID=?"
        );
        delHeader->setString(1, saleID);
        int rows = delHeader->executeUpdate();
        delete delHeader;

        if (rows > 0)
            cout << "\n✅ Refund completed successfully.\n";
        else
            cout << "\n[ERROR] Refund failed.\n";
    }
    catch (sql::SQLException& e) {
        cout << "\n[SQL ERROR] " << e.what() << endl;
    }

    delete conn;
    pauseScreen();
}


void updateSales() {
    system("cls");

    int month, year;
    cout << "Month (0 Back): ";
    if (!inputInt(month) || month == 0) return;

    cout << "Year (0 Back): ";
    if (!inputInt(year) || year == 0) return;

    if (!viewSalesByMonth(month, year))
        return;

    cout << "\nSaleID to update (0 Cancel): ";
    string saleID;
    cin >> saleID;

    if (saleID == "0") return;

    sql::Connection* conn = connectDB();

    try {
        auto ps = conn->prepareStatement(
            "UPDATE sales SET PaymentMethod='Cash' WHERE SaleID=?"
        );
        ps->setString(1, saleID);

        int rows = ps->executeUpdate();
        delete ps;

        if (rows > 0)
            cout << "\nSales updated successfully.\n";
        else
            cout << "\n[ERROR] Sale not found.\n";
    }
    catch (sql::SQLException& e) {
        cout << "\nSQL Error: " << e.what() << endl;
    }

    delete conn;

    cout << "\nPress Enter to continue...";
    cin.ignore(10000, '\n');
    cin.get();
}
void insertSales() {
    system("cls");

    sql::Connection* conn = connectDB();
    string saleID = generateSalesID(conn);

    double grandTotal = 0.0;

    cout << "=========== NEW SALES ===========" << endl;
    cout << "Sale ID: " << saleID << endl;

    /* ===== INSERT HEADER FIRST ===== */
    auto psHeader = conn->prepareStatement(
        "INSERT INTO sales (SaleID, SaleDate, PaymentMethod, ActorID, GrandTotal) "
        "VALUES (?, NOW(), 'Pending', ?, 0)"
    );
    psHeader->setString(1, saleID);
    psHeader->setString(2, currentActorID);
    psHeader->execute();
    delete psHeader;

    while (true) {

        displayInventoryForSales();
        showBackInstruction();

        string itemCode;
        cout << "Enter ItemCode: ";
        if (!inputString(itemCode)) {
            delete conn;
            return;   
        }

        int qty;
        cout << "Quantity: ";
        if (!inputInt(qty) || qty <= 0) {
            delete conn;
            return;   
        }

        try {
            /* ===== PRICE ===== */
            auto psPrice = conn->prepareStatement(
                "SELECT SellPrice FROM product WHERE ItemCode=?"
            );
            psPrice->setString(1, itemCode);
            auto rsPrice = psPrice->executeQuery();
            if (!rsPrice->next())
                throw sql::SQLException("Item not found");

            double originalPrice = rsPrice->getDouble("SellPrice");
            delete rsPrice;
            delete psPrice;

            /* ===== PROMOTION ===== */
            Promotion promo;
            double finalUnitPrice = originalPrice;
            string promoInfo = "None";

            auto psPromo = conn->prepareStatement(
                "SELECT PromotionID FROM promotion "
                "WHERE Status='Active' "
                "AND StartDate<=CURDATE() AND EndDate>=CURDATE() "
                "LIMIT 1"
            );
            auto rsPromo = psPromo->executeQuery();
            if (rsPromo->next() && promo.loadPromotion(rsPromo->getString("PromotionID"))) {
                promoInfo = promo.getPromotionID() + " (" + promo.getPromotionType() + ")";
                finalUnitPrice = promo.applyPromotion(originalPrice, qty);
            }
            delete rsPromo;
            delete psPromo;

            double lineTotal = finalUnitPrice * qty;
            grandTotal += lineTotal;

            /* ===== FIFO ===== */
            int remaining = qty;
            auto psFIFO = conn->prepareStatement(
                "SELECT InventoryID, quantity_in_stock "
                "FROM inventory WHERE ItemCode=? AND quantity_in_stock>0 "
                "ORDER BY InventoryID"
            );
            psFIFO->setString(1, itemCode);
            auto rsFIFO = psFIFO->executeQuery();

            while (rsFIFO->next() && remaining > 0) {
                int deduct = min(rsFIFO->getInt("quantity_in_stock"), remaining);

                auto upd = conn->prepareStatement(
                    "UPDATE inventory SET quantity_in_stock = quantity_in_stock - ? "
                    "WHERE InventoryID=?"
                );
                upd->setInt(1, deduct);
                upd->setString(2, rsFIFO->getString("InventoryID"));
                upd->execute();
                delete upd;

                remaining -= deduct;
            }
            delete rsFIFO;
            delete psFIFO;

            if (remaining > 0)
                throw sql::SQLException("Insufficient stock");

            /* ===== INSERT DETAIL ===== */
            auto psDetail = conn->prepareStatement(
                "INSERT INTO sales_detail "
                "(SaleID, ItemCode, Quantity, UnitPrice, LineTotal) "
                "VALUES (?, ?, ?, ?, ?)"
            );
            psDetail->setString(1, saleID);
            psDetail->setString(2, itemCode);
            psDetail->setInt(3, qty);
            psDetail->setDouble(4, finalUnitPrice);
            psDetail->setDouble(5, lineTotal);
            psDetail->execute();
            delete psDetail;

            cout << "\nItem Added:";
            cout << "\nItemCode : " << itemCode;
            cout << "\nQty      : " << qty;
            cout << "\nPromo    : " << promoInfo;
            cout << "\nLineTotal: RM " << fixed << setprecision(2) << lineTotal << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
            pauseScreen();
            delete conn;
            return;
        }

        cout << "\nAdd another item? (Y/N): ";
        char cont;
        cin >> cont;
        if (toupper(cont) != 'Y') break;
    }

    /* ===== FINAL PAYMENT ===== */
    cout << "\n1. Cash\n2. Card\n3. E-Wallet\nChoose payment: ";
    int pay;
    if (!inputInt(pay)) {
        delete conn;
        return;
    }

    string paymentMethod =
        (pay == 1) ? "Cash" :
        (pay == 2) ? "Card" : "E-Wallet";

    auto psUpdate = conn->prepareStatement(
        "UPDATE sales SET PaymentMethod=?, GrandTotal=? WHERE SaleID=?"
    );
    psUpdate->setString(1, paymentMethod);
    psUpdate->setDouble(2, grandTotal);
    psUpdate->setString(3, saleID);
    psUpdate->execute();
    delete psUpdate;

    cout << "\n=========== RECEIPT ===========" << endl;
    cout << "Sale ID     : " << saleID << endl;
    cout << "Grand Total : RM " << fixed << setprecision(2) << grandTotal << endl;
    cout << "================================\n";

    delete conn;
    pauseScreen();
}

