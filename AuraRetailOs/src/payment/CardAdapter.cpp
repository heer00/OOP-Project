#include <iostream>
#include <string>
#include "payment/CardAdapter.h"

bool CardAdapter::pay(double amount) {
    std::cout << "[CardAdapter] Card payment initiated for Rs." << amount << std::endl;

    // FIX: Actually collect card details from the user instead of silently approving.
    std::string cardNumber, expiry, cvv;
    std::cout << "  Enter card number (16 digits): "; std::cin >> cardNumber;
    std::cout << "  Expiry (MM/YY): ";                std::cin >> expiry;
    std::cout << "  CVV: ";                            std::cin >> cvv;

    // Basic validation — card number must be 16 digits
    if (cardNumber.length() != 16) {
        std::cout << "[Card Gateway SDK] chargeCard -> DECLINED (invalid card number)" << std::endl;
        return false;
    }

    std::cout << "[CardAdapter] Translating pay() -> chargeCard()" << std::endl;
    std::cout << "[Card Gateway SDK] chargeCard(Rs." << amount << ") -> APPROVED" << std::endl;
    return true;
}

bool CardAdapter::refund(const std::string& transactionId) {
    std::cout << "[CardAdapter] Reversing card charge for txn: " << transactionId << std::endl;
    return true;
}

std::string CardAdapter::getStatus() const { return "Card:online"; }