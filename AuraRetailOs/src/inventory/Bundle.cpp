#include <climits>
#include <iostream>
#include <string>
#include "inventory/Bundle.h"

Bundle::Bundle(std::string id, std::string name) : id(id), name(name) {}

void Bundle::addItem(Item* item) { items.push_back(item); }

std::string Bundle::getId()   const { return id; }
std::string Bundle::getName() const { return name; }

// Composite pattern: price = SUM of all children's prices
double Bundle::getPrice() const {
    double total = 0.0;
    for (Item* item : items)
        if (item) total += item->getPrice();
    return total;
}

// Composite pattern: stock = MINIMUM of all children
int Bundle::getStock() const {
    if (items.empty()) return 0;

    int minStock = INT_MAX;
    for (Item* item : items)
        if (item && item->getStock() < minStock)
            minStock = item->getStock();

    return minStock;
}

// Sets stock for all items in the bundle
void Bundle::setStock(int stock) {
    for (Item* item : items)
        if (item) item->setStock(stock);
}