#include <iostream>
#include "inventory/InventoryProxy.h"

InventoryProxy::InventoryProxy(Inventory* realInventory, const std::string& user)
    : realInventory(realInventory), currentUser(user) {}

bool InventoryProxy::authorize(const std::string& operation, const std::string& productId) {
    return true;
}

void InventoryProxy::addItem(Item* item) {
    if (authorize("addItem", item->getId())) {
        realInventory->addItem(item);
        std::cout << "[Inventory] Added: " << item->getName()
                  << " (Stock: " << item->getStock() << ")" << std::endl;
    }
}

Item* InventoryProxy::getItem(const std::string& id) {
    if (authorize("getItem", id))
        return realInventory->getItem(id);
    return nullptr;
}

int InventoryProxy::getStock(const std::string& id) {
    if (authorize("getStock", id)) {
        Item* item = realInventory->getItem(id);
        if (item) return item->getStock();
    }
    return -1;
}

void InventoryProxy::updateStock(const std::string& id, int newStock) {
    if (authorize("updateStock", id)) {
        Item* item = realInventory->getItem(id);
        if (item) item->setStock(newStock);
    }
}