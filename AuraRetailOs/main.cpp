#include "persistence/UserStore.h"

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <cctype>
#include <sstream>

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

// ── Validation helpers ────────────────────────────────────────────────────────

bool isAllDigits(const std::string& s) {
    for (char c : s) if (!std::isdigit(c)) return false;
    return !s.empty();
}

// Validate UPI: must contain '@' and end with "@upi"
bool validateUPI(const std::string& upiId) {
    if (upiId.size() < 5) return false;                        // at least x@upi
    std::string lower = upiId;
    for (char& c : lower) c = std::tolower(c);
    if (lower.size() < 4) return false;
    return lower.substr(lower.size() - 4) == "@upi";
}

// Validate card: 16 digits
bool validateCardNumber(const std::string& num) {
    return num.length() == 16 && isAllDigits(num);
}

// Validate CVV: exactly 3 digits
bool validateCVV(const std::string& cvv) {
    return cvv.length() == 3 && isAllDigits(cvv);
}

// Validate expiry MM/YYYY
// Rules: month 01-12, year >= current year (4 digits), not expired
bool validateExpiry(const std::string& expiry) {
    // Must be exactly MM/YYYY  ->  7 chars
    if (expiry.length() != 7) return false;
    if (expiry[2] != '/') return false;

    std::string mmStr = expiry.substr(0, 2);
    std::string yyyyStr = expiry.substr(3, 4);

    if (!isAllDigits(mmStr) || !isAllDigits(yyyyStr)) return false;

    int month = std::stoi(mmStr);
    int year  = std::stoi(yyyyStr);

    if (month < 1 || month > 12) return false;
    if (yyyyStr.length() != 4)   return false;   // already guaranteed by substr(3,4) + isAllDigits

    // Get current month and year
    std::time_t t = std::time(nullptr);
    std::tm* now  = std::localtime(&t);
    int curYear   = now->tm_year + 1900;
    int curMonth  = now->tm_mon  + 1;

    if (year < curYear) return false;
    if (year == curYear && month < curMonth) return false;

    return true;
}

// Collect & validate UPI details; returns true if accepted
bool collectUPI() {
    while (true) {
        std::string upiId;
        std::cout << "[UPI] Enter UPI ID (must end with @upi, e.g. name@upi): ";
        std::cin >> upiId;
        if (validateUPI(upiId)) {
            std::cout << "[UPI] UPI ID accepted: " << upiId << "\n";
            return true;
        }
        std::cout << "[UPI] Invalid UPI ID. It must end with '@upi' (e.g. rahul@upi). Try again.\n";
    }
}

// Collect & validate Card details; returns true if accepted
bool collectCard() {
    // Card number
    std::string cardNum;
    while (true) {
        std::cout << "[Card] Enter card number (16 digits): ";
        std::cin >> cardNum;
        if (validateCardNumber(cardNum)) break;
        std::cout << "[Card] Invalid card number. Must be exactly 16 digits. Try again.\n";
    }

    // Cardholder name
    std::string cardHolder;
    std::cout << "[Card] Enter cardholder name: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, cardHolder);

    // Expiry date
    std::string expiry;
    while (true) {
        std::cout << "[Card] Enter expiry date (MM/YYYY): ";
        std::cin >> expiry;
        if (!validateExpiry(expiry)) {
            // Give a specific error message
            if (expiry.length() != 7 || expiry[2] != '/') {
                std::cout << "[Card] Invalid format. Use MM/YYYY (e.g. 06/2027).\n";
            } else {
                std::string mmStr   = expiry.substr(0, 2);
                std::string yyyyStr = expiry.substr(3, 4);
                if (!isAllDigits(mmStr) || !isAllDigits(yyyyStr)) {
                    std::cout << "[Card] Month and year must contain digits only.\n";
                } else {
                    int m = std::stoi(mmStr);
                    int y = std::stoi(yyyyStr);
                    if (m < 1 || m > 12) {
                        std::cout << "[Card] Month must be between 01 and 12.\n";
                    } else {
                        std::time_t t  = std::time(nullptr);
                        std::tm* now   = std::localtime(&t);
                        int curYear    = now->tm_year + 1900;
                        if (y < curYear) {
                            std::cout << "[Card] Year must be " << curYear << " or later.\n";
                        } else {
                            std::cout << "[Card] Card has expired. Enter a valid expiry date.\n";
                        }
                    }
                }
            }
            continue;
        }
        break;
    }

    // CVV
    std::string cvv;
    while (true) {
        std::cout << "[Card] Enter CVV (3 digits): ";
        std::cin >> cvv;
        if (validateCVV(cvv)) break;
        std::cout << "[Card] Invalid CVV. Must be exactly 3 digits. Try again.\n";
    }

    std::cout << "[Card] Card details accepted. Processing...\n";
    return true;
}

// ── Wallet login / register ───────────────────────────────────────────────────

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

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    ConfigStore config("data/config.json");
    config.load();
    TransactionLog txLog("data/transactions.csv");

    UserStore userStore("data/users.json");
    userStore.load();

    // Kiosk type selection with validation
    std::string kioskType;
    while (true) {
        std::cout << "\nSelect kiosk type [food / pharmacy / emergency]: ";
        std::cin >> kioskType;
        if (kioskType == "food" || kioskType == "pharmacy" || kioskType == "emergency") break;
        std::cout << "[Error] Invalid kiosk type '" << kioskType
                  << "'. Please enter one of: food, pharmacy, emergency.\n";
    }

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

    // Helper: check if a product ID is valid for this kiosk
    auto isValidProduct = [&](const std::string& pid) -> bool {
        for (auto& p : products)
            if (p.id == pid) return true;
        return false;
    };

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
            // Show available products
            std::cout << "\n[Available Products]\n";
            for (auto& p : products)
                std::cout << "  " << p.id << " | " << p.name << "\n";

            // Product ID with validation loop
            std::string pid;
            while (true) {
                std::cout << "Product ID: ";
                std::cin >> pid;
                if (isValidProduct(pid)) break;
                std::cout << "[Error] Invalid product ID '" << pid
                          << "'. Please enter a valid ID from the list above.\n";
            }

            // Payment method with validation loop
            std::string method;
            while (true) {
                std::cout << "Payment method [card / upi / wallet]: ";
                std::cin >> method;
                if (method == "card" || method == "upi" || method == "wallet") break;
                std::cout << "[Error] Invalid payment method '" << method
                          << "'. Choose one of: card, upi, wallet.\n";
            }

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
                    // Top-up payment method
                    int topupMethod = 0;
                    while (true) {
                        std::cout << "\n[Top-Up] Select payment method:\n"
                                  << "  1. UPI\n"
                                  << "  2. Card\n"
                                  << "Choice: ";
                        topupMethod = readInt();
                        if (topupMethod == 1 || topupMethod == 2) break;
                        std::cout << "[Error] Invalid choice. Enter 1 for UPI or 2 for Card.\n";
                    }

                    bool topupSuccess = false;
                    if (topupMethod == 1) {
                        topupSuccess = collectUPI();
                    } else {
                        topupSuccess = collectCard();
                    }

                    if (topupSuccess) {
                        std::cout << "[Top-Up] Payment of Rs." << topup << " successful. Wallet topped up.\n";
                        UserWallet::getInstance()->topUp(loggedInUser, topup);
                        userStore.save();
                    }
                }

                chosenPayment = new WalletAdapter(loggedInUser);
                ownsPayment   = true;

            } else if (method == "card") {
                collectCard();
                chosenPayment = new CardAdapter();
                ownsPayment   = true;

            } else if (method == "upi") {
                collectUPI();
                chosenPayment = new UPIAdapter();
                ownsPayment   = true;
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