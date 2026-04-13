#include "factory/KioskFactorySimple.h"
#include "factory/PharmacyFactory.h"
#include "factory/FoodFactory.h"
#include "factory/EmergencyFactory.h"
  
KioskFactory* KioskFactorySimple::createFactory(const std::string& type) {
    if (type == "pharmacy")  return new PharmacyFactory();
    if (type == "food")      return new FoodFactory();
    if (type == "emergency") return new EmergencyFactory();
    return new FoodFactory();
}