  // src/command/RefundCommand.cpp
  #include <iostream>
  #include "command/RefundCommand.h"
  
  RefundCommand::RefundCommand(const std::string& transactionId,
                               const std::string& productId,
                               Payment*   payment,
                               Inventory* inventory)
      : transactionId(transactionId),
        productId(productId),
        payment(payment),
        inventory(inventory),
        logMessage("Not executed") {}
  
  void RefundCommand::execute() {
      std::cout << "\n--- RefundCommand: txn=" << transactionId << " ---" << std::endl;
  
      if (!payment->refund(transactionId)) {
          logMessage = "FAILED: Refund rejected for txn [" + transactionId + "]";
          std::cout << "[RefundCommand] " << logMessage << std::endl;
          return;
      }
  
      // Restore stock
      int current = inventory->getStock(productId);
      if (current >= 0) {
          inventory->updateStock(productId, current + 1);
          std::cout << "[Refund] Stock restored for " << productId << std::endl;
          logMessage = "REFUNDED: txn=" + transactionId
                     + " product=" + productId + " stock restored";
      } else {
          logMessage = "REFUNDED: txn=" + transactionId + " (product not in inventory)";
      }
      std::cout << "[RefundCommand] " << logMessage << std::endl;
  }
  
  std::string RefundCommand::getLog() const { return logMessage; }
 
