#include "pricing/StandardPricing.h"

double StandardPricing::computePrice(Item* item) {
    return item->getPrice();
}
