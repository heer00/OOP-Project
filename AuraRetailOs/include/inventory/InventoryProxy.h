#ifndef INVENTORYPROXY_H
#define INVENTORYPROXY_H

#include <string>
#include "inventory/Inventory.h"

class InventoryProxy : public Inventory {
private:
    Inventory* realInventory;
    std::string currentUser;

    bool authorize(const std::string& operation, const std::string& productId);

public:
    InventoryProxy(Inventory* realInventory, const std::string& user);

    void addItem(Item* item) override;
    Item* getItem(const std::string& id) override;
    int getStock(const std::string& id) override;
    void updateStock(const std::string& id, int newStock) override;
};

#endif
