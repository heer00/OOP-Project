#include "pricing/DiscountPricing.h"

DiscountPricing::DiscountPricing(double rate) : discountRate(rate) {}

double DiscountPricing::computePrice(Item* item) {
    return item->getPrice() * (1 - discountRate);
}
