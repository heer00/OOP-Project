#include <iostream>
#include "hardware/RefrigerationModule.h"

RefrigerationModule::RefrigerationModule(KioskInterface* kiosk, int temp)
    : KioskModule(kiosk), temperature(temp) {}

void RefrigerationModule::setTemperature(int temp) {
    temperature = temp;
}

void RefrigerationModule::purchaseItem(const std::string& productId, const std::string& paymentMethod) {
    if (temperature > 8) {
        std::cout << "[RefrigerationModule] BLOCKED -- temperature too high (" << temperature << "C). Product unavailable.\n";
        return;
    }

    std::cout << "[RefrigerationModule] Temperature OK (" << temperature << "C). Forwarding...\n";
    wrappedKiosk->purchaseItem(productId, paymentMethod);
}
