#ifndef WALLETADAPTER_H
#define WALLETADAPTER_H

#include "payment/Payment.h"

class WalletAdapter : public Payment {
public:
    bool pay(double amount) override;
    bool refund(const std::string& transactionId) override;
    std::string getStatus() const override;
};

#endif
