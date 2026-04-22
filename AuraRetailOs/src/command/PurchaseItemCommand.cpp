#include <iostream>
#include "command/PurchaseItemCommand.h"
#include "inventory/Item.h"

PurchaseItemCommand::PurchaseItemCommand(
    const std::string& pid,
    Inventory* inv,
    Payment* pay,
    Dispenser* dis,
    PricingPolicy* pr
)
    : productId(pid),
      inventory(inv),
      payment(pay),
      dispenser(dis),
      pricingPolicy(pr),
      logMessage("Not executed") {}

void PurchaseItemCommand::execute() {
    std::cout << "\n--- PurchaseItemCommand: " << productId << " ---" << std::endl;

    int stock = inventory->getStock(productId);

    // FIX: getStock() returns -1 for a product that doesn't exist at all.
    // The old check (stock <= 0) caught both -1 and 0 and printed "Out of stock"
    // for invalid IDs, which was confusing. Now we check -1 first.
    if (stock < 0) {
        logMessage = "FAILED: Invalid product ID [" + productId + "]";
        std::cout << "[Cmd] " << logMessage << std::endl;
        return;
    }

    if (stock == 0) {
        logMessage = "FAILED: Out of stock [" + productId + "]";
        std::cout << "[Cmd] " << logMessage << std::endl;
        return;
    }

    Item* item = inventory->getItem(productId);
    if (!item) {
        logMessage = "FAILED: Product not found [" + productId + "]";
        std::cout << "[Cmd] " << logMessage << std::endl;
        return;
    }

    memento = InventoryMemento(productId, stock, true);
    std::cout << "[Memento] Snapshot: " << productId
              << " stock=" << stock << std::endl;

    double finalPrice = pricingPolicy->computePrice(item);

    // Step 1: Payment
    if (!payment->pay(finalPrice)) {
        logMessage = "FAILED: Payment declined [" + productId + "]";
        std::cout << "[Cmd] " << logMessage << std::endl;
        return;
    }

    // Step 2: Dispense — rollback if it fails
    if (!dispenser->dispense(productId)) {
        std::cout << "[Memento] ROLLBACK: dispense failed! Reversing payment." << std::endl;
        payment->refund("ROLLBACK-" + productId);
        std::cout << "[Memento] Stock stays at: " << memento.stockBefore << std::endl;
        logMessage = "ROLLED BACK: dispense error, payment reversed [" + productId + "]";
        std::cout << "[Cmd] " << logMessage << std::endl;
        return;
    }

    // Step 3: Commit
    inventory->updateStock(productId, stock - 1);
    logMessage = "SUCCESS: [" + productId + "] Rs." + std::to_string(finalPrice);
    std::cout << "[Cmd] " << logMessage << std::endl;
}

std::string PurchaseItemCommand::getLog() const {
    return logMessage;
}