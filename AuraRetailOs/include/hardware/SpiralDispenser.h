#ifndef SPIRALDISPENSER_H
#define SPIRALDISPENSER_H

#include "hardware/Dispenser.h"

class SpiralDispenser : public Dispenser {
public:
    void dispense(const std::string& productId) override;
    bool isReady() override;
    void calibrate() override;
};

#endif
