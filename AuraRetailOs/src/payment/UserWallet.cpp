#include <iostream>
#include "payment/UserWallet.h"

UserWallet* UserWallet::instance = nullptr;

UserWallet* UserWallet::getInstance() {
    if (!instance)
        instance = new UserWallet();
    return instance;
}

void UserWallet::topUp(const std::string& userId, double amount, bool silent) {
    balanceMap[userId] += amount;

    if (!silent) {
        std::cout << "[UserWallet] " << userId
                  << " +Rs." << amount
                  << " | Balance: Rs." << balanceMap[userId] << std::endl;
    }
}

bool UserWallet::deduct(const std::string& userId, double amount) {
    if (balanceMap[userId] < amount)
        return false;

    balanceMap[userId] -= amount;
    return true;
}

double UserWallet::getBalance(const std::string& userId) {
    return balanceMap[userId];
}