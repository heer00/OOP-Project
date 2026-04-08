#ifndef DISCOUNTPRICING_H
#define DISCOUNTPRICING_H

#include "pricing/PricingPolicy.h"

class DiscountPricing : public PricingPolicy {
private:
    double discountRate;

public:
    DiscountPricing(double rate = 0.10);
    double computePrice(Item* item) override;
};

#endif
