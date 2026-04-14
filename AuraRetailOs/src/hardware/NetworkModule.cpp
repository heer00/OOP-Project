#include <iostream>
#include "hardware/NetworkModule.h"

NetworkModule::NetworkModule(KioskInterface* kiosk, bool offline)
    : KioskModule(kiosk), offline(offline) {}

void NetworkModule::setOffline(bool state) { offline = state; }
bool NetworkModule::isOffline() const { return offline; }

void NetworkModule::purchaseItem(const std::string& productId,
                                 const std::string& paymentMethod) {
    if (offline) {
        queue.push_back({productId, paymentMethod});
        std::cout << "[NetworkModule] OFFLINE -- queued: "
                  << productId << std::endl;
        return;
    }
    wrappedKiosk->purchaseItem(productId, paymentMethod);
}

void NetworkModule::flushQueue() {
    for (auto& q : queue) {
        wrappedKiosk->purchaseItem(q.first, q.second);
    }
    queue.clear();
}
