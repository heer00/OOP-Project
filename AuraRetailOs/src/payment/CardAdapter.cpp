#include <iostream>
#include <string>
#include "payment/CardAdapter.h"

bool CardAdapter::pay(double amount) {
    std::cout << "[CardAdapter] Translating pay() -> chargeCard()" << std::endl;
    std::cout << "[Card Gateway SDK] chargeCard(Rs." << amount << ") -> APPROVED" << std::endl;
    return true;
}

bool CardAdapter::refund(const std::string& transactionId) {
    std::cout << "[CardAdapter] Reversing card charge for txn: " << transactionId << std::endl;
    return true;
}

std::string CardAdapter::getStatus() const { return "Card:online"; }