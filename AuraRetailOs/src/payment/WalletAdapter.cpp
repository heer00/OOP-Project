#include "payment/WalletAdapter.h"
#include <iostream>

WalletAdapter::WalletAdapter(const std::string& userName) : user(userName) {}

bool WalletAdapter::pay(double amount) {
    std::cout << "Payment of " << amount << " done using wallet for user: " << user << "\n";
    return true;
}

bool WalletAdapter::refund(const std::string& transactionId) {
    std::cout << "Refund processed for transaction: " << transactionId << "\n";
    return true;
}

std::string WalletAdapter::getStatus() const {
    return "Wallet:online";
}