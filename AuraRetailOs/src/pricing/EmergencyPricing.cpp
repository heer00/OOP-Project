#include "pricing/EmergencyPricing.h"

EmergencyPricing::EmergencyPricing(double cap) : maxCap(cap) {}

double EmergencyPricing::computePrice(Item* item) {
    return (item->getPrice() < maxCap) ? item->getPrice() : maxCap;
}
