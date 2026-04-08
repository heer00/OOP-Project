#include <iostream>
#include "inventory/InventoryProxy.h"

InventoryProxy::InventoryProxy(Inventory* realInventory, const std::string& user)
    : realInventory(realInventory), currentUser(user) {}

bool InventoryProxy::authorize(const std::string& operation, const std::string& productId) {
    std::cout << "[InventoryProxy] Auth check for user='" << currentUser
              << "' op='" << operation << "' -> GRANTED" << std::endl;
    std::cout << "[InventoryProxy] LOG: user=" << currentUser
              << " op=" << operation << " product=" << productId << std::endl;
    return true;
}

void InventoryProxy::addItem(Item* item) {
    if (authorize("addItem", item->getId()))
        realInventory->addItem(item);
}

Item* InventoryProxy::getItem(const std::string& id) {
    if (authorize("getItem", id))
        return realInventory->getItem(id);
    return nullptr;
}

int InventoryProxy::getStock(const std::string& id) {
    if (authorize("getStock", id))
        return realInventory->getStock(id);
    return -1;
}

void InventoryProxy::updateStock(const std::string& id, int newStock) {
    if (authorize("updateStock", id))
        realInventory->updateStock(id, newStock);
}
