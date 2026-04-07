#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
public:
    virtual std::string getId() const = 0;
    virtual std::string getName() const = 0;
    virtual double getPrice() const = 0;
    virtual int getStock() const = 0;
    virtual ~Item() {}
};

#endif
