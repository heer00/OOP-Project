#ifndef WALLETADAPTER_H
#define WALLETADAPTER_H

#include <string>
#include "payment/Payment.h"

class WalletAdapter : public Payment {
private:
    std::string userId;
    double lastAmount;

public:
    WalletAdapter(const std::string& uid);

    bool pay(double amount) override;
    bool refund(const std::string& reason) override;

    // 🔥 THIS WAS MISSING
    std::string getStatus() const override;
};

#endif