#include "promotion.h"
#include "Database.h"   
#include <iostream>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

Promotion::Promotion() {
    promotionID = "";
    promotionType = "";
    percentageDiscount = 0.0;
    fixedAmountDiscount = 0.0;
    buyX = 0;
    getY = 0;
}

bool Promotion::loadPromotion(string promoID) {
    try {
        sql::Connection* conn = connectDB();
        sql::PreparedStatement* ps;
        sql::ResultSet* rs;

        ps = conn->prepareStatement(
            "SELECT PromotionType, PercentageDiscount, FixedAmountDiscount, BuyX, GetY "
            "FROM promotion WHERE PromotionID = ? AND Status = 'Active'"
        );
        ps->setString(1, promoID);
        rs = ps->executeQuery();

        if (rs->next()) {
            promotionID = promoID;
            promotionType = rs->getString("PromotionType");
            percentageDiscount = rs->getDouble("PercentageDiscount");
            fixedAmountDiscount = rs->getDouble("FixedAmountDiscount");
            buyX = rs->getInt("BuyX");
            getY = rs->getInt("GetY");

            delete rs;
            delete ps;
            delete conn;
            return true;
        }

        delete rs;
        delete ps;
        delete conn;
        return false;
    }
    catch (sql::SQLException& e) {
        cout << "Promotion load error: " << e.what() << endl;
        return false;
    }
}

double Promotion::applyPromotion(double originalPrice, int quantity) {

    // Percentage Discount
    if (promotionType == "Percentage") {
        return originalPrice - (originalPrice * percentageDiscount / 100.0);
    }

    // Fixed Amount Discount
    if (promotionType == "FixedAmount") {
        double finalPrice = originalPrice - fixedAmountDiscount;
        return (finalPrice < 0) ? 0 : finalPrice;
    }

    // Buy X Get Y
    if (promotionType == "BuyXGetY") {
        if (buyX > 0 && getY > 0 && quantity >= buyX) {
            int freeItems = (quantity / (buyX + getY)) * getY;
            double totalPay = (quantity - freeItems) * originalPrice;
            return totalPay / quantity; // return effective unit price
        }
    }

    // No promotion
    return originalPrice;
}

string Promotion::getPromotionID() {
    return promotionID;
}

string Promotion::getPromotionType() {
    return promotionType;
}

double Promotion::calculateDiscount(double originalPrice, int quantity) {

    if (promotionType == "Percentage") {
        return originalPrice * quantity * (percentageDiscount / 100.0);
    }

    if (promotionType == "FixedAmount") {
        return fixedAmountDiscount;
    }

    if (promotionType == "BuyXGetY") {
        if (buyX > 0 && getY > 0 && quantity >= buyX) {
            int freeItems = (quantity / (buyX + getY)) * getY;
            return freeItems * originalPrice;
        }
    }

    return 0.0;
}

