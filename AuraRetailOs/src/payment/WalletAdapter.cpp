#include "payment/WalletAdapter.h"
#include "payment/UserWallet.h"
#include <iostream>

WalletAdapter::WalletAdapter(const std::string& uid)
    : userId(uid), lastAmount(0.0) {}

bool WalletAdapter::pay(double amount) {
    lastAmount = amount;

    bool ok = UserWallet::getInstance()->deduct(userId, amount);

    if (ok)
        std::cout << "[Wallet] Payment successful\n";
    else
        std::cout << "[Wallet] Insufficient balance\n";

    return ok;
}

bool WalletAdapter::refund(const std::string& reason) {
    UserWallet::getInstance()->topUp(userId, lastAmount);

    std::cout << "[Wallet] Refund processed\n";
    return true;
}

// 🔥 REQUIRED IMPLEMENTATION
std::string WalletAdapter::getStatus() const {
    return "Wallet payment processed";
}