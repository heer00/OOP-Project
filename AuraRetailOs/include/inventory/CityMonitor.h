  #ifndef CITYMONITOR_H
  #define CITYMONITOR_H
  #include <iostream>
  #include "inventory/StockObserver.h"
   
  // Concrete Observer: prints city-wide alert to console (in real system: sends network notification)
  class CityMonitor : public StockObserver {
  public:
      void onLowStock(const std::string& productId, int remaining) override {
          std::cout << "[CityMonitor] *** STOCK ALERT *** Product: " << productId
                    << " -- only " << remaining << " units remaining.\n";
          std::cout << "  Notifying supply chain to restock." << std::endl;
      }
  };
  #endif
