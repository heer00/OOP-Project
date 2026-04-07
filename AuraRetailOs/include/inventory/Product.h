#ifndef PRODUCT_H
#define PRODUCT_H

#include "inventory/Item.h"

class Product : public Item {
private:
    std::string id;
    std::string name;
    double price;
    int stock;

public:
    Product(std::string id, std::string name, double price, int stock);
    std::string getId() const override;
    std::string getName() const override;
    double getPrice() const override;
    int getStock() const override;
};

#endif
