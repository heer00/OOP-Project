
  #ifndef KIOSKINTERFACE_H
  #define KIOSKINTERFACE_H
  
  #include <string>
  
  class KioskInterface {
  public:
      virtual void purchaseItem(const std::string& productId,
                                const std::string& paymentMethod) = 0;
      virtual void refundTransaction(const std::string& transactionId) = 0;
      virtual void restockInventory(const std::string& productId, int qty) = 0;
      virtual void runDiagnostics() = 0;
      virtual ~KioskInterface() {}
  };
  
  #endif