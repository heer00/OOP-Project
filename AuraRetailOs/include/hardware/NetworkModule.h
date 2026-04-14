#ifndef NETWORKMODULE_H
#define NETWORKMODULE_H

#include <vector>
#include <string>
#include "hardware/KioskModule.h"

class NetworkModule : public KioskModule {
private:
    bool offline;
    std::vector<std::pair<std::string,std::string>> queue;

public:
    NetworkModule(KioskInterface* kiosk, bool offline = false);
    void setOffline(bool state);
    bool isOffline() const;
    void flushQueue();
    void purchaseItem(const std::string& productId,
                      const std::string& paymentMethod) override;
};

#endif
