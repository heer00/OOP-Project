#include <iostream>
#include <string>

#include "inventory/Product.h"
#include "inventory/Inventory.h"
#include "inventory/RealInventory.h"
#include "inventory/InventoryProxy.h"

#include "payment/Payment.h"
#include "payment/CardAdapter.h"
#include "payment/UPIAdapter.h"

#include "pricing/PricingPolicy.h"
#include "pricing/StandardPricing.h"
#include "pricing/DiscountPricing.h"

// Helper: perform a purchase using given pricing strategy and payment method
void performPurchase(Inventory& inv, const std::string& itemId,
                     PricingPolicy* pricing, Payment* payment) {
    std::cout << "\n--- Purchase Request: " << itemId << " ---" << std::endl;

    Item* item = inv.getItem(itemId);
    if (!item) {
        std::cout << "[ERROR] Item not found: " << itemId << std::endl;
        return;
    }
    if (item->getStock() <= 0) {
        std::cout << "[ERROR] Out of stock: " << item->getName() << std::endl;
        return;
    }

    // Strategy Pattern: compute price
    double finalPrice = pricing->computePrice(item);

    // Adapter Pattern: payment
    bool success = payment->pay(finalPrice);

    if (success) {
        int newStock = item->getStock() - 1;
        inv.updateStock(itemId, newStock);
        std::cout << "[Kiosk] Purchase SUCCESS. Remaining stock: "
                  << item->getStock() << std::endl;
    } else {
        std::cout << "[Kiosk] Purchase FAILED." << std::endl;
    }
}

int main() {
    std::cout << "========== Aura Retail OS - Subtask 2 Simulation ==========" << std::endl;

    // --- Setup Inventory (Proxy Pattern) ---
    RealInventory realInventory;
    InventoryProxy inventory(&realInventory, "admin");

    inventory.addItem(new Product("P001", "Mineral Water",  20.0, 5));
    inventory.addItem(new Product("P002", "Chocolate Bar",  50.0, 3));
    inventory.addItem(new Product("P003", "Aspirin Pack",  120.0, 2));

    // --- Pricing Strategies ---
    StandardPricing standard;
    DiscountPricing discount(0.15);   // 15% off

    // --- Payment Methods ---
    CardAdapter cardPay;
    UPIAdapter  upiPay;

    // --- Simulations ---
    performPurchase(inventory, "P001", &standard, &cardPay);  // Rs.20,  Card
    performPurchase(inventory, "P002", &discount, &upiPay);   // Rs.42.5, UPI
    performPurchase(inventory, "P003", &standard, &upiPay);   // Rs.120, UPI
    performPurchase(inventory, "P003", &standard, &cardPay);  // Rs.120, Card
    performPurchase(inventory, "P003", &standard, &cardPay);  // out of stock

    std::cout << "\n========== Simulation Complete ==========" << std::endl;
    return 0;
}