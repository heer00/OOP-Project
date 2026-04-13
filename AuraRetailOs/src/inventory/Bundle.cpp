#include <climits>
  #include <iostream>
  #include "inventory/Bundle.h"
  
  Bundle::Bundle(std::string id, std::string name)
      : id(id), name(name) {}
  
  void Bundle::addItem(Item* item) {
      items.push_back(item);
  }
  
  std::string Bundle::getId()   const { return id;   }
  std::string Bundle::getName() const { return name; }
  
  // Composite: price = sum of all children
  double Bundle::getPrice() const {
      double total = 0.0;
      for (Item* item : items) total += item->getPrice();
      return total;
  }
  
  // Composite: stock = minimum of all children (can't sell if any item is out)
  int Bundle::getStock() const {
      if (items.empty()) return 0;
      int minStock = INT_MAX;
      for (Item* item : items) {
          if (item->getStock() < minStock) minStock = item->getStock();
      }
      return minStock;
  }
  
  void Bundle::setStock(int stock) {
      // Propagate new stock to all children proportionally (set all to same value)
      for (Item* item : items) item->setStock(stock);
  }
