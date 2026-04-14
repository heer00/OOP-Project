#include <iostream>
#include "hardware/SolarModule.h"

SolarModule::SolarModule(KioskInterface* kiosk, int battery)
    : KioskModule(kiosk), batteryLevel(battery) {}

void SolarModule::setBatteryLevel(int level) { batteryLevel = level; }
int SolarModule::getBatteryLevel() const { return batteryLevel; }

void SolarModule::runDiagnostics() {
    std::cout << "[SolarModule] Battery: " << batteryLevel << "%" << std::endl;
    if (batteryLevel < 20) {
        std::cout << "[SolarModule] WARNING -- low battery, kiosk restricted." << std::endl;
    } else {
        wrappedKiosk->runDiagnostics();
    }
}

void SolarModule::purchaseItem(const std::string& productId,
                               const std::string& paymentMethod) {
    if (batteryLevel < 20) {
        std::cout << "[SolarModule] BLOCKED -- battery too low ("
                  << batteryLevel << "%)." << std::endl;
        return;
    }
    wrappedKiosk->purchaseItem(productId, paymentMethod);
}
