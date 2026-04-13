#include <iostream>
#include "factory/EmergencyFactory.h"
#include "hardware/ConveyorDispenser.h"
#include "payment/WalletAdapter.h"
#include "inventory/RealInventory.h"
#include "inventory/InventoryProxy.h"
#include "inventory/Product.h"
#include "pricing/EmergencyPricing.h"
  
Dispenser* EmergencyFactory::createDispenser() {
    std::cout << "[EmergencyFactory] Creating ConveyorDispenser\n";
    return new ConveyorDispenser();
}
Payment* EmergencyFactory::createPayment() {
    std::cout << "[EmergencyFactory] Creating WalletAdapter\n";
    return new WalletAdapter();
}
  
Inventory* EmergencyFactory::createInventory() {
    std::cout << "[EmergencyFactory] Creating Emergency Inventory\n";
    RealInventory* real = new RealInventory();
    real->addItem(new Product("EMER001", "First Aid Kit",  80.0, 20));
    real->addItem(new Product("EMER002", "Water Bottle",   15.0, 50));
    real->addItem(new Product("EMER003", "Energy Bar",     25.0, 30));
    return new InventoryProxy(real, "emergency-system");
}
  
PricingPolicy* EmergencyFactory::createPricingPolicy() {
    std::cout << "[EmergencyFactory] Creating EmergencyPricing (cap=50)\n";
    return new EmergencyPricing(50.0);
}