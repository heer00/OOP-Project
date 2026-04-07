#ifndef BUNDLE_H
#define BUNDLE_H

#include <vector>
#include "inventory/Item.h"

class Bundle : public Item {
private:
    std::string id;
    std::string name;
    std::vector<Item*> items;

public:
    Bundle(std::string id, std::string name);
    void addItem(Item* item);

    std::string getId() const override;
    std::string getName() const override;
    double getPrice() const override;
    int getStock() const override;
};

#endif
