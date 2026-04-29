#ifndef KIOSKMODULE_H
#define KIOSKMODULE_H

#include "core/KioskInterface.h"

class KioskModule : public KioskInterface {
protected:
    KioskInterface* wrappedKiosk;

public:
    KioskModule(KioskInterface* kiosk) : wrappedKiosk(kiosk) {}

    void purchaseItem(const std::string& productId, const std::string& paymentMethod) override {
        wrappedKiosk->purchaseItem(productId, paymentMethod);
    }

    void refundTransaction(const std::string& transactionId) override {
        wrappedKiosk->refundTransaction(transactionId);
    }

    void restockInventory(const std::string& productId, int qty) override {
        wrappedKiosk->restockInventory(productId, qty);
    }

    void runDiagnostics() override {
        wrappedKiosk->runDiagnostics();
    }

    KioskInterface* getWrappedKiosk() const { return wrappedKiosk; }

    virtual ~KioskModule() {}

};

#endif
