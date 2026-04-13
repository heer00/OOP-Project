 #ifndef EMERGENCYFACTORY_H
  #define EMERGENCYFACTORY_H
  
  #include "factory/KioskFactory.h"
  
  class EmergencyFactory : public KioskFactory {
  public:
      Dispenser*     createDispenser()     override;
      Payment*       createPayment()       override;
      Inventory*     createInventory()     override;
      PricingPolicy* createPricingPolicy() override;
  };
  
  #endif