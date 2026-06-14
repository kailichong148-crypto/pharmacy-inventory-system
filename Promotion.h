#ifndef PROMOTION_H
#define PROMOTION_H

#include <string>

using namespace std;

class Promotion {
private:
    string promotionID;
    string promotionType;          // Percentage / FixedAmount / BuyXGetY
    double percentageDiscount;     // %
    double fixedAmountDiscount;    // RM
    int buyX;
    int getY;

public:
    /* ===== Constructor ===== */
    Promotion();

    /* ===== Load Promotion ===== */
    bool loadPromotion(string promoID);

    /* ===== Apply Promotion (returns FINAL UNIT PRICE) ===== */
    double applyPromotion(double originalPrice, int quantity);

    /* ===== Calculate Discount Amount (for receipt display) ===== */
    double calculateDiscount(double originalPrice, int quantity);

    /* ===== Getters ===== */
    string getPromotionID();
    string getPromotionType();
};

#endif


