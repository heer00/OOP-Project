#ifndef STANDARDPRICING_H
#define STANDARDPRICING_H

#include "pricing/PricingPolicy.h"

class StandardPricing : public PricingPolicy {
public:
    double computePrice(Item* item) override;
};

#endif
