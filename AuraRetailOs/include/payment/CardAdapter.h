#ifndef CARDADAPTER_H
#define CARDADAPTER_H

#include "payment/Payment.h"

class CardAdapter : public Payment {
public:
    bool pay(double amount) override;
    bool refund(const std::string& transactionId) override;
    std::string getStatus() const override;
};

#endif
