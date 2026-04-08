#include "payment/UPIAdapter.h"

bool UPIAdapter::pay(double amount) {
    return true;
}

bool UPIAdapter::refund(const std::string& transactionId) {
    return true;
}

std::string UPIAdapter::getStatus() const {
    return "UPI: online";
}
