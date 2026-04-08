#ifndef UPIADAPTER_H
#define UPIADAPTER_H

#include "payment/Payment.h"

class UPIAdapter : public Payment {
public:
    bool pay(double amount) override;
    bool refund(const std::string& transactionId) override;
    std::string getStatus() const override;
};

#endif
