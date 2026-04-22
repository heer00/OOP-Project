#include <iostream>
#include <fstream>
#include "persistence/UserStore.h"
#include "payment/UserWallet.h"

UserStore::UserStore(const std::string& path) : filePath(path) {}

void UserStore::load() {
    std::ifstream file(filePath);
    if (!file.is_open()) return;

    users.clear();
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.find("\"username\"") == std::string::npos) continue;

        auto get = [&](const std::string& key) -> std::string {
            auto p = line.find("\"" + key + "\":");
            if (p == std::string::npos) return "";
            p += key.size() + 3;

            while (p < line.size() && line[p] == ' ') p++;

            size_t q = line.size();
            for (char ch : {',', '}'}) {
                auto idx = line.find(ch, p);
                if (idx != std::string::npos && idx < q) q = idx;
            }

            std::string v = line.substr(p, q - p);

            while (!v.empty() && v.back() == ' ') v.pop_back();

            if (v.size() >= 2 && v.front() == '"')
                v = v.substr(1, v.size() - 2);

            return v;
        };

        std::string uname  = get("username");
        std::string pwd    = get("password");
        std::string balStr = get("balance");

        if (uname.empty() || pwd.empty()) continue;

        double bal = balStr.empty() ? 0.0 : std::stod(balStr);

        users[uname] = {uname, pwd, bal};

        // silent load
        UserWallet::getInstance()->topUp(uname, bal, true);
    }

    file.close();
}

void UserStore::save() {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cout << "[UserStore] ERROR: Cannot write file\n";
        return;
    }

    file << "[\n";
    bool first = true;

    for (auto& pair : users) {
        double liveBal = UserWallet::getInstance()->getBalance(pair.first);

        if (!first) file << ",\n";

        file << "  {\"username\": \"" << pair.second.username << "\""
             << ", \"password\": \"" << pair.second.password << "\""
             << ", \"balance\": "    << liveBal << "}";

        first = false;
    }

    file << "\n]\n";
    file.close();

    std::cout << "[UserStore] Saved\n";
}

bool UserStore::authenticate(const std::string& username,
                             const std::string& password) const {
    auto it = users.find(username);
    if (it == users.end()) return false;
    return it->second.password == password;
}

bool UserStore::exists(const std::string& username) const {
    return users.count(username) > 0;
}

bool UserStore::registerUser(const std::string& username,
                             const std::string& password,
                             double initialBalance) {
    if (exists(username)) return false;

    users[username] = {username, password, initialBalance};

    if (initialBalance > 0.0)
        UserWallet::getInstance()->topUp(username, initialBalance);

    std::cout << "[UserStore] Registered: " << username << std::endl;

    return true;
}