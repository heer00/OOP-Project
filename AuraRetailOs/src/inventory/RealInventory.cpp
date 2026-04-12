#include "inventory/RealInventory.h"

void RealInventory::addItem(Item* item) {
    items[item->getId()] = item;
}

Item* RealInventory::getItem(const std::string& id) {
    if (items.count(id)) return items[id];
    return nullptr;
}

int RealInventory::getStock(const std::string& id) {
    if (items.count(id)) return items[id]->getStock();
    return -1;
}

void RealInventory::updateStock(const std::string& id, int newStock) {
    if (items.count(id)) {
        items[id]->setStock(newStock);
    }
}