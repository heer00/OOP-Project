#ifndef PAYMENTPOOL_H
#define PAYMENTPOOL_H

#include <queue>
#include <set>
#include "payment/Payment.h"

class PaymentPool {
private:
    std::queue<Payment*> available;
    std::set<Payment*> inUse;

public:
    PaymentPool();
    Payment* acquire();
    void release(Payment* payment);
};

#endif
