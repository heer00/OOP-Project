#ifndef CONVEYORDISPENSER_H
#define CONVEYORDISPENSER_H

#include "hardware/Dispenser.h"

class ConveyorDispenser : public Dispenser {
private:
    bool ready;

public:
    ConveyorDispenser();
    void calibrate() override;
    bool isReady() const override;
    bool dispense(const std::string& productId) override;
};

#endif
