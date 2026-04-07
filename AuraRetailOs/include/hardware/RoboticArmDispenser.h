#ifndef ROBOTICARMDISPENSER_H
#define ROBOTICARMDISPENSER_H

#include "hardware/Dispenser.h"

class RoboticArmDispenser : public Dispenser {
public:
    void dispense(const std::string& productId) override;
    bool isReady() override;
    void calibrate() override;
};

#endif
