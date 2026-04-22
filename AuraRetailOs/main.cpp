#include "persistence/UserStore.h"

#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include "core/KioskInterface.h"
#include "core/Kiosk.h"
#include "core/KioskBuilder.h"
#include "core/CentralRegistry.h"

#include "factory/KioskFactorySimple.h"
#include "factory/KioskFactory.h"

#include "hardware/SolarModule.h"
#include "hardware/NetworkModule.h"

#include "persistence/ConfigStore.h"
#include "persistence/TransactionLog.h"
#include "persistence/InventoryStore.h"
#include "persistence/UserStore.h"

#include "command/PurchaseItemCommand.h"
#include "command/RestockCommand.h"

#include "inventory/Inventory.h"
#include "inventory/RealInventory.h"
#include "inventory/InventoryProxy.h"
#include "inventory/CityMonitor.h"
#include "inventory/Product.h"

#include "payment/CardAdapter.h"
#include "payment/UPIAdapter.h"
#include "payment/WalletAdapter.h"
#include "payment/UserWallet.h"

struct ProductInfo { std::string id; std::string name; };

std::vector<ProductInfo> getProductList(const std::string& type) {
    if (type == "food")
        return {{"FOOD001","Chips Pack"},{"FOOD002","Cold Drink"},{"FOOD003","Chocolate Bar"}};
    if (type == "pharmacy")
        return {{"MED001","Aspirin Pack"},{"MED002","Vitamin C"},{"MED003","Bandage Roll"}};
    if (type == "emergency")
        return {{"EMER001","First Aid Kit"},{"EMER002","Water Bottle"},{"EMER003","Energy Bar"}};
    return {};
}

void printStock(const std::string& id, const std::string& name, int stock) {
    std::cout << "  " << id << " | " << name << " | ";
    if      (stock <  0) std::cout << "Product not found in this kiosk\n";
    else if (stock == 0) std::cout << "Out of Stock\n";
    else if (stock <= 3) std::cout << stock << " units  *** LOW STOCK ***\n";
    else                 std::cout << stock << " units\n";
}

int readInt() {
    int val;
    while (!(std::cin >> val)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a number: ";
    }
    return val;
}

std::string walletLogin(UserStore& userStore) {
    std::cout << "\n[Wallet] 1. Login   2. Register\nChoice: ";
    int wChoice = readInt();

    if (wChoice == 1) {
        std::string uname, pwd;
        std::cout << "Username: "; std::cin >> uname;
        std::cout << "Password: "; std::cin >> pwd;

        if (!userStore.authenticate(uname, pwd)) {
            std::cout << "[Wallet] Login failed: incorrect username or password.\n";
            return "";
        }
        double bal = UserWallet::getInstance()->getBalance(uname);
        std::cout << "[Wallet] Welcome, " << uname
                  << "! Current balance: Rs." << bal << "\n";
        return uname;

    } else if (wChoice == 2) {
        std::string uname, pwd;
        std::cout << "Enter Your Username: "; std::cin >> uname;

        if (userStore.exists(uname)) {
            std::cout << "[Wallet] Username '" << uname
                      << "' is already taken. Try a different one.\n";
            return "";
        }

        std::cout << "Choose a password: "; std::cin >> pwd;

        double deposit = 0.0;
        std::cout << "Initial wallet deposit (Rs.): "; std::cin >> deposit;

        userStore.registerUser(uname, pwd, deposit);
        std::cout << "[Wallet] Account created! Welcome, " << uname << ".\n";
        return uname;

    } else {
        std::cout << "[Wallet] Cancelled.\n";
        return "";
    }
}

int main() {
    ConfigStore config("data/config.json");
    config.load();
    TransactionLog txLog("data/transactions.csv");

    UserStore userStore("data/users.json");
    userStore.load();

    std::string kioskType;
    std::cout << "\nSelect kiosk type [food / pharmacy / emergency]: ";
    std::cin >> kioskType;

    KioskFactory* factory = KioskFactorySimple::createFactory(kioskType);

    RealInventory* realInv = new RealInventory();
    InventoryStore invStore("data/inventory.json");
    invStore.load(realInv);

    CityMonitor cityMonitor;
    InventoryProxy* proxy = new InventoryProxy(
        realInv, kioskType + "-system", &cityMonitor, 3);

    Kiosk* baseKiosk = KioskBuilder()
        .addDispenser(factory->createDispenser())
        .addPayment(factory->createPayment())
        .addInventory(proxy)
        .addPricingPolicy(factory->createPricingPolicy())
        .build();

    KioskInterface* kiosk = baseKiosk;
    kiosk = new SolarModule(kiosk, 100);
    kiosk = new NetworkModule(kiosk, false);

    std::vector<ProductInfo> products = getProductList(kioskType);

    int choice = 0;
    while (choice != 6) {
        std::cout << "\n--- Aura Kiosk Menu [" << kioskType << "] ---\n"
                  << "1. Buy item\n"
                  << "2. View stock\n"
                  << "3. Restock item\n"
                  << "4. Run diagnostics\n"
                  << "5. Check wallet balance\n"
                  << "6. Exit\n"
                  << "Choice: ";
        choice = readInt();

        // ── 1. BUY ───────────────────────────────────────────────
        if (choice == 1) {
            std::string pid, method;
            std::cout << "Product ID: "; std::cin >> pid;
            std::cout << "Payment method [card / upi / wallet]: "; std::cin >> method;

            Payment* chosenPayment = nullptr;
            bool ownsPayment = false;

            if (method == "wallet") {
                std::string loggedInUser = walletLogin(userStore);
                if (loggedInUser.empty()) {
                    std::cout << "[Payment] Wallet login failed. Purchase cancelled.\n";
                    continue;
                }

                double bal = UserWallet::getInstance()->getBalance(loggedInUser);
                std::cout << "[Wallet] Balance: Rs." << bal << "\n";

                double topup = 0.0;
                std::cout << "Top-up before purchase? (Rs., 0 to skip): ";
                std::cin >> topup;
                if (topup > 0.0) {
                    UserWallet::getInstance()->topUp(loggedInUser, topup);
                    userStore.save();
                }

                chosenPayment = new WalletAdapter(loggedInUser);
                ownsPayment   = true;

            } else if (method == "card") {
                std::cout << "[Card] Enter card number (16 digits): ";
                std::string cardNum; std::cin >> cardNum;
                std::cout << "[Card] Enter CVV: ";
                std::string cvv; std::cin >> cvv;
                std::cout << "[Card] Details accepted.\n";
                chosenPayment = new CardAdapter();
                ownsPayment   = true;

            } else if (method == "upi") {
                std::cout << "[UPI] Enter UPI ID (e.g. name@upi): ";
                std::string upiId; std::cin >> upiId;
                std::cout << "[UPI] ID accepted: " << upiId << "\n";
                chosenPayment = new UPIAdapter();
                ownsPayment   = true;

            } else {
                std::cout << "[Payment] Unknown method '" << method
                          << "'. Valid options: card, upi, wallet.\n";
                continue;
            }

            PurchaseItemCommand cmd(pid,
                baseKiosk->getInventory(),
                chosenPayment,
                baseKiosk->getDispenser(),
                baseKiosk->getPricingPolicy());
            cmd.execute();
            txLog.append(cmd.getLog());

            if (cmd.getLog().find("SUCCESS") != std::string::npos) {
                invStore.save(realInv);
                if (method == "wallet") userStore.save();
            }

            if (ownsPayment) delete chosenPayment;
        }

        // ── 2. VIEW STOCK ─────────────────────────────────────────
        else if (choice == 2) {
            std::cout << "\n[Stock Status - " << kioskType << " kiosk]\n";
            std::cout << "  Enter product ID (or 'all'): ";
            std::string input; std::cin >> input;

            if (input == "all") {
                for (auto& p : products) {
                    int s = realInv->getStock(p.id);
                    printStock(p.id, p.name, s);
                }
            } else {
                int s = realInv->getStock(input);
                std::string name = "Unknown";
                for (auto& p : products)
                    if (p.id == input) { name = p.name; break; }
                printStock(input, name, s);
            }
        }

        // ── 3. RESTOCK ───────────────────────────────────────────
        else if (choice == 3) {
            std::string pid; int qty;
            std::cout << "Product ID: "; std::cin >> pid;
            std::cout << "Quantity: "; qty = readInt();

            RestockCommand cmd(pid, qty, baseKiosk->getInventory());
            cmd.execute();
            txLog.append(cmd.getLog());

            if (cmd.getLog().find("RESTOCK") != std::string::npos) {
                int s = realInv->getStock(pid);
                std::string name = "Unknown";
                for (auto& p : products)
                    if (p.id == pid) { name = p.name; break; }
                std::cout << "[Restock] Final stock: ";
                printStock(pid, name, s);
                invStore.save(realInv);
            }
        }

        // ── 4. DIAGNOSTICS ───────────────────────────────────────
        else if (choice == 4) {
            kiosk->runDiagnostics();
        }

        // ── 5. CHECK WALLET BALANCE ───────────────────────────────
        else if (choice == 5) {
            std::cout << "\n[Wallet Balance Check]\n";
            std::string uname, pwd;
            std::cout << "Username: "; std::cin >> uname;
            std::cout << "Password: "; std::cin >> pwd;

            if (!userStore.authenticate(uname, pwd)) {
                std::cout << "[Wallet] Login failed: incorrect username or password.\n";
            } else {
                double bal = UserWallet::getInstance()->getBalance(uname);
                std::cout << "[Wallet] Account: " << uname
                          << " | Balance: Rs." << bal << "\n";
            }
        }

        else if (choice != 6) {
            std::cout << "Invalid choice. Enter 1-6.\n";
        }
    }

    userStore.save();
    std::cout << "[Aura Kiosk] Shutting down.\n";
    delete factory;
    return 0;
}