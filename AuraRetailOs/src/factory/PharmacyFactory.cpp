#include <iostream>
#include "factory/PharmacyFactory.h"
#include "hardware/RoboticArmDispenser.h"
#include "payment/CardAdapter.h"
#include "inventory/RealInventory.h"
#include "inventory/InventoryProxy.h"
#include "inventory/Product.h"
#include "pricing/StandardPricing.h"
  
Dispenser* PharmacyFactory::createDispenser() {
    std::cout << "[PharmacyFactory] Creating RoboticArmDispenser\n";
    return new RoboticArmDispenser();
}
  
Payment* PharmacyFactory::createPayment() {
    std::cout << "[PharmacyFactory] Creating CardAdapter\n";
    return new CardAdapter();
}
  
Inventory* PharmacyFactory::createInventory() {
    std::cout << "[PharmacyFactory] Creating Pharmacy Inventory\n";
    RealInventory* real = new RealInventory();
    real->addItem(new Product("MED001", "Aspirin Pack",   120.0, 50));
    real->addItem(new Product("MED002", "Vitamin C",       80.0, 30));
    real->addItem(new Product("MED003", "Bandage Roll",    40.0, 100));
    return new InventoryProxy(real, "pharmacy-system");
}
  
PricingPolicy* PharmacyFactory::createPricingPolicy() {
    std::cout << "[PharmacyFactory] Creating StandardPricing\n";
    return new StandardPricing();
}