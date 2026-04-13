  #ifndef INVENTORYSTORE_H
  #define INVENTORYSTORE_H
  
  #include <string>
  #include "inventory/RealInventory.h"
  
  class InventoryStore {
  private:
      std::string filePath;
  
  public:
      InventoryStore(const std::string& path);
      void load(RealInventory* inventory);
      void save(RealInventory* inventory);
  };
  
  #endif
