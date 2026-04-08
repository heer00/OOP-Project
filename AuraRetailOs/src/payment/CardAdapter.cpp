#include "payment/CardAdapter.h"

bool CardAdapter::pay(double amount) {
    return true;
}

bool CardAdapter::refund(const std::string& transactionId) {
    return true;
}

std::string CardAdapter::getStatus() const {
    return "Card: online";
}
