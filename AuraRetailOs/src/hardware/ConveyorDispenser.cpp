#include <iostream>
#include "hardware/ConveyorDispenser.h"

ConveyorDispenser::ConveyorDispenser() : ready(false) {}

void ConveyorDispenser::calibrate() {
    ready = true;
    std::cout << "[ConveyorDispenser] Belt aligned and tensioned." << std::endl;
}

bool ConveyorDispenser::isReady() const { return ready; }

bool ConveyorDispenser::dispense(const std::string& productId) {
    if (!ready) calibrate();
    std::cout << "[ConveyorDispenser] Belt moving -- delivering product: "
              << productId << std::endl;
    return true;
}

