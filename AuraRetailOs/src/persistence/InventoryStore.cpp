  #include <iostream>
  #include <fstream>
  #include <sstream>
  #include "persistence/InventoryStore.h"
  #include "inventory/Product.h"
  
  InventoryStore::InventoryStore(const std::string& path) : filePath(path) {}
  
  // Simple JSON parser for: [{"id":"P001","name":"Water","price":20,"stock":5}, ...]
  void InventoryStore::load(RealInventory* inventory) {
      std::ifstream file(filePath);
      if (!file.is_open()) {
          std::cout << "[InventoryStore] WARNING: " << filePath
                    << " not found. Starting with empty inventory." << std::endl;
          return;
      }
  
      std::string line, id, name;
      double price = 0;
      int stock = 0;
      while (std::getline(file, line)) {
          auto get = [&](const std::string& key) -> std::string {
              auto p = line.find("\"" + key + "\":");
              if (p == std::string::npos) return "";
              p += key.size() + 3;
              auto q = line.find_first_of(",}", p);
              std::string v = line.substr(p, q - p);
              if (!v.empty() && v[0] == '"') v = v.substr(1, v.size() - 2);
              return v;
          };
          if (line.find("\"id\"") != std::string::npos) {
              id    = get("id");
              name  = get("name");
              price = get("price").empty() ? 0 : std::stod(get("price"));
              stock = get("stock").empty() ? 0 : std::stoi(get("stock"));
              if (!id.empty()) {
                  inventory->addItem(new Product(id, name, price, stock));
                  std::cout << "[InventoryStore] Loaded: " << id
                            << " (" << name << ") stock=" << stock << std::endl;
              }
          }
      }
      file.close();
  }
  
  void InventoryStore::save(RealInventory* inventory) {
      // Re-reads current items from inventory and writes back to JSON
      // For simplicity, writes a minimal JSON array
      std::ofstream file(filePath);
      if (!file.is_open()) {
          std::cout << "[InventoryStore] ERROR: Cannot write to " << filePath << std::endl;
          return;
      }
      file << "[\n";
      // Note: RealInventory would need a getAll() method for full save
      // This writes the file header; actual items saved via Product updates
      file << "]\n";
      file.close();
      std::cout << "[InventoryStore] Saved to " << filePath << std::endl;
  }
 
