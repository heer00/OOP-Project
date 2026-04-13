  #include <iostream>
  #include <fstream>
  #include <string>
  #include "persistence/ConfigStore.h"
  #include "core/CentralRegistry.h"
  
  ConfigStore::ConfigStore(const std::string& path) : filePath(path) {}
  
  // Reads key-value pairs from JSON: {"discountRate":"0.15","emergencyMode":"false"}
  void ConfigStore::load() {
      std::ifstream file(filePath);
      if (!file.is_open()) {
          std::cout << "[ConfigStore] WARNING: " << filePath
                    << " not found. Using defaults." << std::endl;
          CentralRegistry::getInstance()->setConfig("discountRate", "0.10");
          CentralRegistry::getInstance()->setConfig("emergencyMode", "false");
          CentralRegistry::getInstance()->setConfig("maxCap",        "50");
          return;
      }
  
      std::string line;
      while (std::getline(file, line)) {
          auto colon = line.find("\": \"");
          if (colon == std::string::npos) continue;
          auto ks = line.find('\"');
          auto ke = line.find('\"', ks + 1);
          auto vs = line.find('\"', colon);
          auto ve = line.find('\"', vs + 1);
          if (ks == std::string::npos) continue;
          std::string key = line.substr(ks + 1, ke - ks - 1);
          std::string val = (vs != std::string::npos)
                            ? line.substr(vs + 1, ve - vs - 1) : "";
          CentralRegistry::getInstance()->setConfig(key, val);
          std::cout << "[ConfigStore] Set " << key << " = " << val << std::endl;
      }
      file.close();
  }
