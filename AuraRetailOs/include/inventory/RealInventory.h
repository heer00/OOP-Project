#ifndef REALINVENTORY_H
#define REALINVENTORY_H

#include <map>
#include "inventory/Inventory.h"

class RealInventory : public Inventory {
private:
    std::map<std::string, Item*> items;

public:
    void addItem(Item* item) override;
    Item* getItem(const std::string& id) override;
    int getStock(const std::string& id) override;
    void updateStock(const std::string& id, int newStock) override;
};

#endif
