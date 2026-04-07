#include <iostream>
#include "hardware/RoboticArmDispenser.h"

void RoboticArmDispenser::dispense(const std::string& productId) {
    std::cout << "[RoboticArmDispenser] Arm picking and placing product: " << productId << std::endl;
}

bool RoboticArmDispenser::isReady() {
    return false;
}

void RoboticArmDispenser::calibrate() {
    std::cout << "[RoboticArmDispenser] Arm calibrated and zeroed.\n";
}
