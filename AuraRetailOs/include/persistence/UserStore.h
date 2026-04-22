#ifndef USERSTORE_H
#define USERSTORE_H

#include <string>
#include <map>

struct User {
    std::string username;
    std::string password;
    double balance;
};

class UserStore {
private:
    std::string filePath;
    std::map<std::string, User> users;

public:
    UserStore(const std::string& path);

    void load();
    void save();

    bool authenticate(const std::string& username,
                      const std::string& password) const;

    bool exists(const std::string& username) const;

    bool registerUser(const std::string& username,
                      const std::string& password,
                      double initialBalance);
};

#endif