#ifndef INVENTORY_H
#define INVENTORY_H

#include "inventory/Item.h"
#include <string>

class Inventory {
public:
    virtual void addItem(Item* item) = 0;
    virtual Item* getItem(const std::string& id) = 0;
    virtual int getStock(const std::string& id) = 0;
    virtual void updateStock(const std::string& id, int newStock) = 0;

    virtual ~Inventory() {}
};

#endif