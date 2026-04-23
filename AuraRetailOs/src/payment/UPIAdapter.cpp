#include <iostream>
#include <string>
#include "payment/UPIAdapter.h"

bool UPIAdapter::pay(double amount) {
    std::cout << "[UPIAdapter] Translating pay() -> initiateUPITransaction()" << std::endl;
    std::cout << "[UPI SDK] initiateUPITransaction(Rs." << amount << ") -> SUCCESS" << std::endl;
    return true;
}

bool UPIAdapter::refund(const std::string& transactionId) {
    std::cout << "[UPIAdapter] Refunding UPI transaction: " << transactionId << std::endl;
    return true;
}

std::string UPIAdapter::getStatus() const { return "UPI:online"; }