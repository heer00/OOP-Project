#include "payment/PaymentPool.h"

PaymentPool::PaymentPool() {}

Payment* PaymentPool::acquire() {
    return nullptr;
}

void PaymentPool::release(Payment* payment) {}
