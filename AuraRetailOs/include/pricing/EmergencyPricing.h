#ifndef EMERGENCYPRICING_H
#define EMERGENCYPRICING_H

#include "pricing/PricingPolicy.h"

class EmergencyPricing : public PricingPolicy {
private:
    double maxCap;

public:
    EmergencyPricing(double cap = 50.0);
    double computePrice(Item* item) override;
};

#endif
