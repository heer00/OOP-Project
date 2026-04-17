  #ifndef INVENTORYPROXY_H
  #define INVENTORYPROXY_H
  #include <string>
  #include "inventory/Inventory.h"
  #include "inventory/StockObserver.h"   // NEW
   
  // Proxy pattern: intercepts all inventory access for auth + logging + alerts
  class InventoryProxy : public Inventory {
  private:
      Inventory*     realInventory;
      std::string    currentUser;
      StockObserver* observer;           // NEW: optional alert subscriber
      int            lowStockThreshold;  // NEW: fire alert when stock <= this
   
      bool authorize(const std::string& op, const std::string& pid);
  public:
      // observer and threshold are optional -- default = no alerts
      InventoryProxy(Inventory* inv, const std::string& user,
                     StockObserver* obs = nullptr, int threshold = 5);
      void setObserver(StockObserver* obs);   // attach/replace observer at runtime
      void addItem(Item* item)                               override;
      Item* getItem(const std::string& id)                   override;
      int   getStock(const std::string& id)                  override;
      void  updateStock(const std::string& id, int newStock) override;
  };
  #endif
