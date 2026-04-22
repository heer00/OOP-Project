#include <iostream>
#include <string>
#include "payment/UPIAdapter.h"

bool UPIAdapter::pay(double amount) {
    std::cout << "[UPIAdapter] UPI payment initiated for Rs." << amount << std::endl;

    // FIX: Actually collect UPI ID from the user instead of silently approving.
    std::string upiId;
    std::cout << "  Enter UPI ID (e.g. name@upi): "; std::cin >> upiId;

    // Basic validation — must contain '@'
    if (upiId.find('@') == std::string::npos) {
        std::cout << "[UPI SDK] initiateUPITransaction -> FAILED (invalid UPI ID)" << std::endl;
        return false;
    }

    std::cout << "[UPIAdapter] Translating pay() -> initiateUPITransaction()" << std::endl;
    std::cout << "[UPI SDK] initiateUPITransaction(Rs." << amount << ") -> SUCCESS" << std::endl;
    return true;
}

bool UPIAdapter::refund(const std::string& transactionId) {
    std::cout << "[UPIAdapter] Refunding UPI transaction: " << transactionId << std::endl;
    return true;
}

std::string UPIAdapter::getStatus() const { return "UPI:online"; }