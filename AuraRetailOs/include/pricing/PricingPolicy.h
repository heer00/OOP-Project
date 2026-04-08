#ifndef PRICINGPOLICY_H
#define PRICINGPOLICY_H

#include "inventory/Item.h"

class PricingPolicy {
public:
    virtual double computePrice(Item* item) = 0;
    virtual ~PricingPolicy() {}
};

#endif
