#include "core/Kiosk.h"
#include "inventory/Inventory.h"
#include "payment/Payment.h"
#include "hardware/Dispenser.h"
#include "pricing/PricingPolicy.h"
#include <iostream>

Kiosk::Kiosk(Inventory* i, Payment* p, Dispenser* d, PricingPolicy* pr)
    : inventory(i), payment(p), dispenser(d), pricing(pr) {}

void Kiosk::purchaseItem(const std::string& productId,
                         const std::string& paymentMethod) {

    int stock = inventory->getStock(productId);

    if (stock <= 0) {
        std::cout << "Out of stock\n";
        return;
    }

Item* item = inventory->getItem(productId);
double price = pricing->computePrice(item);

// OPTIONAL: use paymentMethod if your Payment class supports it
    if (!payment->pay(price)) {
        std::cout << "Payment failed\n";
        return;
    }

    dispenser->dispense(productId);
    inventory->updateStock(productId, stock - 1);

    std::cout << "Purchase successful using " << paymentMethod << "\n";
}

Inventory* Kiosk::getInventory() const {
    return inventory;
}

Payment* Kiosk::getPayment() const {
    return payment;
}

Dispenser* Kiosk::getDispenser() const {
    return dispenser;
}

PricingPolicy* Kiosk::getPricingPolicy() const {
    return pricing;
}
void Kiosk::refundTransaction(const std::string& transactionId) {
    std::cout << "Refunding transaction: " << transactionId << "\n";
}

void Kiosk::restockInventory(const std::string& productId, int quantity) {
    inventory->updateStock(productId, inventory->getStock(productId) + quantity);
    std::cout << "Restocked " << productId << " with quantity " << quantity << "\n";
}

void Kiosk::runDiagnostics() {
    std::cout << "Running diagnostics...\n";
}