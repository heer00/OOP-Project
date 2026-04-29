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
#include "command/RefundCommand.h"

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

// ── Validation helpers ─────────────────────────────────────────────────────
bool isAllDigits(const std::string& s) {
    for (char c : s) if (!std::isdigit(c)) return false;
    return !s.empty();
}
bool validateUPI(const std::string& upiId) {
    std::string lower = upiId;
    for (char& c : lower) c = std::tolower(c);
    return lower.size() >= 5 && lower.substr(lower.size() - 4) == "@upi";
}
bool validateCardNumber(const std::string& num) {
    return num.length() == 16 && isAllDigits(num);
}
bool validateCVV(const std::string& cvv) {
    return cvv.length() == 3 && isAllDigits(cvv);
}
bool validateExpiry(const std::string& expiry) {
    if (expiry.length() != 7 || expiry[2] != '/') return false;
    std::string mm = expiry.substr(0, 2), yyyy = expiry.substr(3, 4);
    if (!isAllDigits(mm) || !isAllDigits(yyyy)) return false;
    int month = std::stoi(mm), year = std::stoi(yyyy);
    if (month < 1 || month > 12) return false;
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    int cy = now->tm_year + 1900, cm = now->tm_mon + 1;
    return year > cy || (year == cy && month >= cm);
}

bool collectUPI() {
    while (true) {
        std::string upiId;
        std::cout << "[UPI] Enter UPI ID (must end with @upi, e.g. name@upi): ";
        std::cin >> upiId;
        if (validateUPI(upiId)) {
            std::cout << "[UPI] UPI ID accepted: " << upiId << "\n";
            return true;
        }
        std::cout << "[UPI] Invalid. Must end with '@upi'. Try again.\n";
    }
}

bool collectCard() {
    std::string cardNum;
    while (true) {
        std::cout << "[Card] Enter card number (16 digits): ";
        std::cin >> cardNum;
        if (validateCardNumber(cardNum)) break;
        std::cout << "[Card] Must be exactly 16 digits. Try again.\n";
    }
    std::string cardHolder;
    std::cout << "[Card] Enter cardholder name: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, cardHolder);
    std::string expiry;
    while (true) {
        std::cout << "[Card] Enter expiry (MM/YYYY): ";
        std::cin >> expiry;
        if (validateExpiry(expiry)) break;
        std::cout << "[Card] Invalid expiry. Use MM/YYYY (e.g. 06/2027).\n";
    }
    std::string cvv;
    while (true) {
        std::cout << "[Card] Enter CVV (3 digits): ";
        std::cin >> cvv;
        if (validateCVV(cvv)) break;
        std::cout << "[Card] Must be exactly 3 digits. Try again.\n";
    }
    std::cout << "[Card] Card details accepted.\n";
    return true;
}

// Issue 3: wallet PIN validation (4 digits)
bool validateWalletPIN(const std::string& pin) {
    return pin.length() == 4 && isAllDigits(pin);
}

// ── Wallet login/register ──────────────────────────────────────────────────
// Returns logged-in username and sets outPin for refund verification
std::string walletLogin(UserStore& userStore, std::string& outPin) {
    std::cout << "\n[Wallet] 1. Login   2. Register\nChoice: ";
    int wChoice = readInt();

    if (wChoice == 1) {
        std::string uname, pwd;
        std::cout << "Username: "; std::cin >> uname;
        std::cout << "Password: "; std::cin >> pwd;
        if (!userStore.authenticate(uname, pwd)) {
            std::cout << "[Wallet] Login failed.\n";
            return "";
        }
        // Issue 3: ask wallet PIN to verify
        std::string pin;
        while (true) {
            std::cout << "[Wallet] Enter 4-digit wallet PIN: ";
            std::cin >> pin;
            if (validateWalletPIN(pin)) break;
            std::cout << "[Wallet] PIN must be exactly 4 digits. Try again.\n";
        }
        outPin = pin;
        double bal = UserWallet::getInstance()->getBalance(uname);
        std::cout << "[Wallet] Welcome, " << uname << "! Balance: Rs." << bal << "\n";
        return uname;

    } else if (wChoice == 2) {
        std::string uname, pwd;
        std::cout << "Username: "; std::cin >> uname;
        if (userStore.exists(uname)) {
            std::cout << "[Wallet] Username taken.\n";
            return "";
        }
        std::cout << "Password: "; std::cin >> pwd;

        // Issue 3: set a 4-digit PIN on registration
        std::string pin;
        while (true) {
            std::cout << "[Wallet] Set a 4-digit wallet PIN: ";
            std::cin >> pin;
            if (validateWalletPIN(pin)) break;
            std::cout << "[Wallet] PIN must be exactly 4 digits.\n";
        }
        outPin = pin;

        double deposit = 0.0;
        std::cout << "Initial deposit (Rs.): "; std::cin >> deposit;
        userStore.registerUser(uname, pwd, deposit);
        std::cout << "[Wallet] Account created! Welcome, " << uname << ".\n";
        return uname;
    }
    return "";
}

// Issue 3: top-up flow — ask card/upi, validate, then credit wallet
void doTopUp(const std::string& loggedInUser, UserStore& userStore) {
    double topup = 0.0;
    std::cout << "Top-up amount (Rs., 0 to skip): "; std::cin >> topup;
    if (topup <= 0.0) return;

    int topupMethod = 0;
    while (true) {
        std::cout << "[Top-Up] Method: 1. UPI   2. Card\nChoice: ";
        topupMethod = readInt();
        if (topupMethod == 1 || topupMethod == 2) break;
        std::cout << "Invalid. Enter 1 or 2.\n";
    }
    bool ok = (topupMethod == 1) ? collectUPI() : collectCard();
    if (ok) {
        UserWallet::getInstance()->topUp(loggedInUser, topup);
        userStore.save();
        std::cout << "[Top-Up] Rs." << topup << " added. New balance: Rs."
                  << UserWallet::getInstance()->getBalance(loggedInUser) << "\n";
    }
}

// Issue 4: refund — ask original payment method, validate, refund to same method
void doRefund(TransactionLog& txLog, Inventory* inventory,
              UserStore& userStore, InventoryStore& invStore,
              RealInventory* realInv,
              const std::vector<ProductInfo>& products) {
    std::cout << "\n--- Refund ---\n";
    std::string pid;
    while (true) {
        std::cout << "Product ID to refund: "; std::cin >> pid;
        int s = inventory->getStock(pid);
        if (s >= 0) break;
        std::cout << "[Refund] Product not found in this kiosk. Try again.\n";
    }

    std::string method;
    while (true) {
        std::cout << "Original payment method [card / upi / wallet]: ";
        std::cin >> method;
        if (method == "card" || method == "upi" || method == "wallet") break;
        std::cout << "[Refund] Invalid. Enter card, upi, or wallet.\n";
    }

    Payment* refundPayment = nullptr;
    bool ownsPayment = false;
    std::string txnId = "REF-" + pid;

    if (method == "wallet") {
        // Issue 4: verify wallet identity before refund
        std::string uname, pwd;
        std::cout << "[Refund-Wallet] Username: "; std::cin >> uname;
        std::cout << "[Refund-Wallet] Password: "; std::cin >> pwd;
        if (!userStore.authenticate(uname, pwd)) {
            std::cout << "[Refund] Wallet authentication failed. Refund cancelled.\n";
            return;
        }
        std::string pin;
        while (true) {
            std::cout << "[Refund-Wallet] Enter 4-digit PIN: "; std::cin >> pin;
            if (validateWalletPIN(pin)) break;
            std::cout << "[Refund-Wallet] Invalid PIN. Try again.\n";
        }
        std::cout << "[Refund] Refund will be credited back to wallet: " << uname << "\n";
        refundPayment = new WalletAdapter(uname);
        ownsPayment = true;

    } else if (method == "card") {
        // Issue 4: validate card details to confirm ownership before refunding
        std::cout << "[Refund-Card] Please re-enter original card details to verify:\n";
        collectCard();
        std::cout << "[Refund] Refund will be credited back to your card.\n";
        refundPayment = new CardAdapter();
        ownsPayment = true;

    } else if (method == "upi") {
        // Issue 4: validate UPI ID before refunding
        std::cout << "[Refund-UPI] Please re-enter original UPI ID to verify:\n";
        collectUPI();
        std::cout << "[Refund] Refund will be sent back to your UPI ID.\n";
        refundPayment = new UPIAdapter();
        ownsPayment = true;
    }

    RefundCommand cmd(txnId, pid, refundPayment, inventory);
    cmd.execute();
    txLog.append(cmd.getLog());

    if (cmd.getLog().find("REFUNDED") != std::string::npos) {
        invStore.save(realInv);
        if (method == "wallet") userStore.save();
        std::cout << "[Refund] Stock restored. ";
        std::string name = "Unknown";
        for (auto& p : products) if (p.id == pid) { name = p.name; break; }
        printStock(pid, name, realInv->getStock(pid));
    }

    if (ownsPayment) delete refundPayment;
}

// ── Admin stock alert check ───────────────────────────────────────────────
void checkAndPrintAlerts(RealInventory* realInv,
                         const std::vector<ProductInfo>& products) {
    bool anyAlert = false;
    for (auto& p : products) {
        int s = realInv->getStock(p.id);
        if (s == 0) {
            std::cout << "  [ALERT] OUT OF STOCK: " << p.id
                      << " | " << p.name << "\n";
            anyAlert = true;
        } else if (s > 0 && s <= 3) {
            std::cout << "  [ALERT] LOW STOCK:    " << p.id
                      << " | " << p.name << " | only " << s << " left\n";
            anyAlert = true;
        }
    }
    if (!anyAlert)
        std::cout << "  All products have healthy stock levels.\n";
}

// ── Main ──────────────────────────────────────────────────────────────────
int main() {
    ConfigStore config("data/config.json");
    config.load();
    TransactionLog txLog("data/transactions.csv");

    UserStore userStore("data/users.json");
    userStore.load();

    std::string kioskType;
    while (true) {
        std::cout << "\nSelect kiosk type [food / pharmacy / emergency]: ";
        std::cin >> kioskType;
        if (kioskType=="food"||kioskType=="pharmacy"||kioskType=="emergency") break;
        std::cout << "[Error] Invalid type. Enter food, pharmacy, or emergency.\n";
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

    SolarModule* solar = new SolarModule(baseKiosk, 100);
    NetworkModule* network = new NetworkModule(solar, false);
    KioskInterface* kiosk = network;


    std::vector<ProductInfo> products = getProductList(kioskType);

    auto isValidProduct = [&](const std::string& pid) -> bool {
        for (auto& p : products) if (p.id == pid) return true;
        return false;
    };

    int choice = 0;
    while (choice != 7) {
        std::cout << "\n--- Aura Kiosk Menu [" << kioskType << "] ---\n"
                  << "1. Buy item\n"
                  << "2. View stock\n"
                  << "3. Wallet (top-up / balance)\n"
                  << "4. Refund\n"
                  << "5. Run diagnostics\n"
                  << "6. Admin panel\n"
                  << "7. Exit\n"
                  << "Choice: ";
        choice = readInt();

        // ── 1. BUY ─────────────────────────────────────────────────
        if (choice == 1) {
            std::cout << "\n[Available Products]\n";
            for (auto& p : products)
                std::cout << "  " << p.id << " | " << p.name << "\n";

            std::string pid;
            while (true) {
                std::cout << "Product ID: "; std::cin >> pid;
                if (isValidProduct(pid)) break;
                std::cout << "[Error] Invalid ID. Choose from list above.\n";
            }

            std::string method;
            while (true) {
                std::cout << "Payment method [card / upi / wallet]: ";
                std::cin >> method;
                if (method=="card"||method=="upi"||method=="wallet") break;
                std::cout << "[Error] Invalid. Enter card, upi, or wallet.\n";
            }

            Payment* chosenPayment = nullptr;
            bool ownsPayment = false;

            if (method == "wallet") {
                std::string walletPin;
                std::string loggedInUser = walletLogin(userStore, walletPin);
                if (loggedInUser.empty()) {
                    std::cout << "[Payment] Wallet login failed. Purchase cancelled.\n";
                    continue;
                }
                // Issue 3: offer top-up before paying
                doTopUp(loggedInUser, userStore);
                chosenPayment = new WalletAdapter(loggedInUser);
                ownsPayment = true;

            } else if (method == "card") {
                collectCard();
                chosenPayment = new CardAdapter();
                ownsPayment = true;

            } else {
                collectUPI();
                chosenPayment = new UPIAdapter();
                ownsPayment = true;
            }

            PurchaseItemCommand cmd(pid,
                kiosk, // Decorated kiosk
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

        // ── 2. VIEW STOCK ───────────────────────────────────────────
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

        // ── 3. WALLET ───────────────────────────────────────────────
        else if (choice == 3) {
            std::cout << "\n[Wallet] 1. Top-up   2. View balance\nChoice: ";
            int wc = readInt();

            if (wc == 1) {
                std::string walletPin;
                std::string user = walletLogin(userStore, walletPin);
                if (!user.empty()) doTopUp(user, userStore);

            } else if (wc == 2) {
                // Issue 3: view balance — login required
                std::string uname, pwd;
                std::cout << "Username: "; std::cin >> uname;
                std::cout << "Password: "; std::cin >> pwd;
                if (!userStore.authenticate(uname, pwd)) {
                    std::cout << "[Wallet] Login failed.\n";
                } else {
                    std::string pin;
                    while (true) {
                        std::cout << "[Wallet] Enter 4-digit PIN: ";
                        std::cin >> pin;
                        if (validateWalletPIN(pin)) break;
                        std::cout << "[Wallet] Invalid PIN.\n";
                    }
                    double bal = UserWallet::getInstance()->getBalance(uname);
                    std::cout << "[Wallet] Account: " << uname
                              << " | Balance: Rs." << bal << "\n";
                }
            }
        }

        // ── 4. REFUND ───────────────────────────────────────────────
        else if (choice == 4) {
            // Issue 4: validated refund routed back to original payment method
            doRefund(txLog, baseKiosk->getInventory(), userStore,
                     invStore, realInv, products);
        }

        // ── 5. DIAGNOSTICS ──────────────────────────────────────────
        else if (choice == 5) {
            kiosk->runDiagnostics();
        }

        // ── 6. ADMIN PANEL ──────────────────────────────────────────
        // Issue 1: admin view stock, restock, stock alerts
        else if (choice == 6) {
            std::cout << "\n--- Admin Panel ---\n"
                      << "A. View all stock\n"
                      << "B. Restock item\n"
                      << "C. Stock alerts\n"
                      << "D. View transaction log\n"
                      << "S. Simulation Settings\n"
                      << "E. Back\n"

                      << "Choice: ";
            std::string adminChoice; std::cin >> adminChoice;

            if (adminChoice == "A" || adminChoice == "a") {
                // Issue 1: admin view all stock with clear alert labels
                std::cout << "\n[Admin] Stock Status — " << kioskType << " kiosk\n";
                for (auto& p : products) {
                    int s = realInv->getStock(p.id);
                    printStock(p.id, p.name, s);
                }

            } else if (adminChoice == "B" || adminChoice == "b") {
                // Issue 1: admin restock
                std::string pid; int qty;
                std::cout << "[Admin] Product ID: "; std::cin >> pid;
                std::cout << "[Admin] Quantity to add: "; qty = readInt();

                RestockCommand cmd(pid, qty, baseKiosk->getInventory());
                cmd.execute();
                txLog.append(cmd.getLog());

                if (cmd.getLog().find("RESTOCK") != std::string::npos) {
                    int s = realInv->getStock(pid);
                    std::string name = "Unknown";
                    for (auto& p : products)
                        if (p.id == pid) { name = p.name; break; }
                    std::cout << "[Admin] Final stock: ";
                    printStock(pid, name, s);
                    invStore.save(realInv);
                }

            } else if (adminChoice == "C" || adminChoice == "c") {
                // Issue 1: stock alert summary for admin
                std::cout << "\n[Admin] Stock Alert Summary\n";
                checkAndPrintAlerts(realInv, products);

            } else if (adminChoice == "D" || adminChoice == "d") {
                // Show transaction log
                std::cout << "\n[Admin] Transaction Log\n";
                auto lines = txLog.readAll();
                if (lines.empty()) {
                    std::cout << "  No transactions recorded yet.\n";
                }
            } else if (adminChoice == "S" || adminChoice == "s") {
                std::cout << "\n--- Simulation Settings ---\n"
                          << "1. Set Solar Battery %\n"
                          << "2. Toggle Network Status\n"
                          << "Choice: ";
                int sChoice = readInt();
                if (sChoice == 1) {
                    std::cout << "Enter Battery % (0-100): ";
                    int b = readInt();
                    solar->setBatteryLevel(b);
                    std::cout << "[Sim] Battery set to " << b << "%\n";
                } else if (sChoice == 2) {
                    network->setOffline(!network->isOffline());
                    std::cout << "[Sim] Network is now " << (network->isOffline() ? "OFFLINE" : "ONLINE") << "\n";
                }
            }
        }


        else if (choice != 7) {
            std::cout << "Invalid choice. Enter 1-7.\n";
        }
    }

    userStore.save();
    std::cout << "[Aura Kiosk] Shutting down.\n";
    delete factory;
    return 0;
}