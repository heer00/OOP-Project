#include "payment/WalletAdapter.h"

bool WalletAdapter::pay(double amount) {
    return true;
}

bool WalletAdapter::refund(const std::string& transactionId) {
    return true;
}

std::string WalletAdapter::getStatus() const {
    return "Wallet: online";
}
