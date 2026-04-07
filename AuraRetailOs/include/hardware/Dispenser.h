#ifndef DISPENSER_H
#define DISPENSER_H

#include <string>

class Dispenser {
public:
    virtual void dispense(const std::string& productId) = 0;
    virtual bool isReady() = 0;
    virtual void calibrate() = 0;
    virtual ~Dispenser() {}
};

#endif
