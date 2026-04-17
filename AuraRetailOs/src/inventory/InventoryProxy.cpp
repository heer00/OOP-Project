#include <iostream>
  #include "inventory/InventoryProxy.h"
   
  InventoryProxy::InventoryProxy(Inventory* inv, const std::string& user,
                                 StockObserver* obs, int threshold)
      : realInventory(inv), currentUser(user), observer(obs), lowStockThreshold(threshold) {}
   
  void InventoryProxy::setObserver(StockObserver* obs) { observer = obs; }
   
  bool InventoryProxy::authorize(const std::string& op, const std::string& pid) {
      std::cout << "[InventoryProxy] AUTH: user='" << currentUser << "' op='" << op
                << "' product='" << pid << "' -> GRANTED" << std::endl;
      return true;
  }
   
  void InventoryProxy::addItem(Item* item) {
      if (authorize("addItem", item->getId())) realInventory->addItem(item);
  }
   
  Item* InventoryProxy::getItem(const std::string& id) {
      if (authorize("getItem", id)) return realInventory->getItem(id);
      return nullptr;
  }
   
  int InventoryProxy::getStock(const std::string& id) {
      if (authorize("getStock", id)) return realInventory->getStock(id);
      return -1;
  }
   
  void InventoryProxy::updateStock(const std::string& id, int newStock) {
      if (authorize("updateStock", id)) {
          realInventory->updateStock(id, newStock);
          // Observer pattern: fires automatically -- no changes needed in commands or kiosk
          if (observer && newStock >= 0 && newStock <= lowStockThreshold)
              observer->onLowStock(id, newStock);
      }
  }
