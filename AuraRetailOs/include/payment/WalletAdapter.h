#ifndef WALLETADAPTER_H
#define WALLETADAPTER_H

#include "payment/Payment.h"
#include <string>

class WalletAdapter : public Payment {
private:
    std::string user;

public:
    WalletAdapter(const std::string& userName);

    bool pay(double amount) override;
    bool refund(const std::string& transactionId) override;
    std::string getStatus() const override;
};

#endif  