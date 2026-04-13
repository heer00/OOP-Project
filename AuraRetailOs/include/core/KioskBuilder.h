 #ifndef KIOSKBUILDER_H
  #define KIOSKBUILDER_H
  
  class Dispenser;
  class Payment;
  class Inventory;
  class PricingPolicy;
  class Kiosk;
  
  class KioskBuilder {
  private:
      Dispenser*     dispenser;
      Payment*       payment;
      Inventory*     inventory;
      PricingPolicy* pricingPolicy;
  
  public:
      KioskBuilder();
      KioskBuilder& addDispenser(Dispenser* d);
      KioskBuilder& addPayment(Payment* p);
      KioskBuilder& addInventory(Inventory* i);
      KioskBuilder& addPricingPolicy(PricingPolicy* pr);
      Kiosk* build();
  };
  
  #endif