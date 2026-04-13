#ifndef KIOSK_H
  #define KIOSK_H
  
  #include <string>
  #include "core/KioskInterface.h"
  
  class Dispenser;
  class Payment;
  class Inventory;
  class PricingPolicy;
  
  class Kiosk : public KioskInterface {
  private:
      Dispenser*     dispenser;
      Payment*       payment;
      Inventory*     inventory;
      PricingPolicy* pricingPolicy;
  
  public:
      Kiosk(Dispenser* d, Payment* p, Inventory* i, PricingPolicy* pr);
  
      Dispenser*     getDispenser()     const;
      Payment*       getPayment()       const;
      Inventory*     getInventory()     const;
      PricingPolicy* getPricingPolicy() const;
  
      void purchaseItem(const std::string& productId,
                        const std::string& paymentMethod) override;
      void refundTransaction(const std::string& transactionId) override;
      void restockInventory(const std::string& productId, int qty) override;
      void runDiagnostics() override;
  };
  
  #endif