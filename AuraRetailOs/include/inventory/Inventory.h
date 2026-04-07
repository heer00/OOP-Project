#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include "inventory/Item.h"

class Inventory {
public:
    virtual void addItem(Item* item) = 0;
    virtual Item* getItem(const std::string& id) = 0;
    virtual int getStock(const std::string& id) = 0;
    virtual void updateStock(const std::string& id, int qty) = 0;
    virtual ~Inventory() {}
};

#endif
