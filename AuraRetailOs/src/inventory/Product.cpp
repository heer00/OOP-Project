#include "inventory/Product.h"

Product::Product(const std::string& id, const std::string& name, double price, int stock)
    : id(id), name(name), price(price), stock(stock) {}

std::string Product::getId() const { return id; }
std::string Product::getName() const { return name; }
double Product::getPrice() const { return price; }
int Product::getStock() const { return stock; }
void Product::setStock(int stock) { this->stock = stock; }

Product* Product::clone() const {
    return new Product(*this);
}
