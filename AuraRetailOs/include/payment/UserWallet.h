#ifndef USERWALLET_H
#define USERWALLET_H

#include <string>
#include <map>

class UserWallet {
private:
    static UserWallet* instance;
    std::map<std::string, double> balanceMap;

    UserWallet() {}

public:
    static UserWallet* getInstance();

    // MUST match cpp (3 params)
    void topUp(const std::string& userId, double amount, bool silent = false);

    bool deduct(const std::string& userId, double amount);

    double getBalance(const std::string& userId);
};

#endif