#ifndef MANAGE_INVENTORY_H
#define MANAGE_INVENTORY_H

#include <string>

// ===== 主菜单 =====
void manageInventoryMenu();

// ===== Inventory =====
void checkInventoryLevel();
void checkInventoryExpire();

// ===== Reorder =====
void reorderLowStockItems();

// ===== Supplier helper =====
void displaySuppliersSimple();
std::string getLastSupplierForItem(const std::string& itemCode);



#endif
