#ifndef SOLARMODULE_H
#define SOLARMODULE_H

#include "hardware/KioskModule.h"

class SolarModule : public KioskModule {
private:
    int batteryLevel;

public:
    SolarModule(KioskInterface* kiosk, int battery = 100);
    void setBatteryLevel(int level);
    int getBatteryLevel() const;
    void runDiagnostics() override;
    void purchaseItem(const std::string& productId,
                      const std::string& paymentMethod) override;
};

#endif
