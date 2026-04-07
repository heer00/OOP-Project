#ifndef REFRIGERATIONMODULE_H
#define REFRIGERATIONMODULE_H

#include "hardware/KioskModule.h"

class RefrigerationModule : public KioskModule {
private:
    int temperature;

public:
    RefrigerationModule(KioskInterface* kiosk, int temp);
    void setTemperature(int temp);
    void purchaseItem(const std::string& productId, const std::string& paymentMethod) override;
};

#endif
