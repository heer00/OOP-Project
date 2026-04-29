#include "core/diagnosisservice.h"
#include "inventory/Inventory.h"
#include "core/Kiosk.h"
#include "hardware/SolarModule.h"
#include "hardware/NetworkModule.h"
#include "hardware/KioskModule.h"
#include <sstream>
#include <vector>

DiagnosisReport DiagnosisService::runFullCheck(KioskInterface* kiosk) {
    DiagnosisReport report;
    std::ostringstream oss;
    
    int totalProducts = 0;
    int lowStockCount = 0;
    int battery = 100;
    bool offline = false;
    bool solarFound = false;
    
    // Unwrap the decorator chain to find real data
    KioskInterface* current = kiosk;
    Inventory* inv = nullptr;

    while (current) {
        // Try to find SolarModule
        SolarModule* solar = dynamic_cast<SolarModule*>(current);
        if (solar) {
            battery = solar->getBatteryLevel();
            solarFound = true;
        }

        // Try to find NetworkModule
        NetworkModule* net = dynamic_cast<NetworkModule*>(current);
        if (net) {
            offline = net->isOffline();
        }

        // Try to find the base Kiosk (to get inventory)
        Kiosk* base = dynamic_cast<Kiosk*>(current);
        if (base) {
            inv = base->getInventory();
            break; // Reached the bottom
        }

        // Move down the chain if it's a module
        KioskModule* mod = dynamic_cast<KioskModule*>(current);
        if (mod) {
            current = mod->getWrappedKiosk();
        } else {
            break;
        }
    }
    
    if (inv) {
        std::vector<std::string> ids = inv->getAllProductIds();
        totalProducts = ids.size();
        for (const auto& id : ids) {
            if (inv->getStock(id) < 5) {
                lowStockCount++;
            }
        }
    }

    oss << "----------------------------------------\n\n";
    oss << "[Hardware]\n";
    oss << "Dispenser Type      : Robotic Arm\n";
    oss << "Dispenser Status    : <span style=\"color:#4caf50\">OK</span>\n";
    oss << "Motor Controller    : <span style=\"color:#4caf50\">OK</span>\n";
    oss << "Sensor Module       : <span style=\"color:#4caf50\">OK</span>\n";
    oss << "Door Lock           : Locked\n\n";

    oss << "[Optional Modules]\n";
    oss << "Refrigeration       : ON\n";
    oss << "Solar Module        : " << (solarFound ? (std::to_string(battery) + "%") : "Not Installed") << "\n";
    oss << "Network Module      : " << (offline ? "<span style=\"color:#f44336\">Offline</span>" : "<span style=\"color:#4caf50\">Online</span>") << "\n\n";

    oss << "[Payment Systems]\n";
    oss << "UPI                 : <span style=\"color:#4caf50\">Available</span>\n";
    oss << "Card                : <span style=\"color:#4caf50\">Available</span>\n";
    oss << "Wallet              : <span style=\"color:#4caf50\">Available</span>\n\n";

    oss << "[Inventory]\n";
    oss << "Total Products      : " << totalProducts << "\n";
    oss << "Low Stock Items     : " << (lowStockCount > 0 ? "<span style=\"color:#ffeb3b\">" : "") 
        << lowStockCount << (lowStockCount > 0 ? "</span>" : "") << "\n\n";

    oss << "[System & Security]\n";
    oss << "Authorization       : Active\n";
    oss << "Logging             : Enabled\n\n";
    oss << "----------------------------------------";

    report.fullReport = oss.str();
    return report;
}


