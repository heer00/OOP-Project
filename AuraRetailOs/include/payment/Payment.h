#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>

class Payment {
public:
    virtual bool pay(double amount) = 0;
    virtual bool refund(const std::string& transactionId) = 0;
    virtual std::string getStatus() const = 0;
    virtual ~Payment() {}
};

#endif
