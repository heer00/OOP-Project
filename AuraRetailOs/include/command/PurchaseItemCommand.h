#ifndef PURCHASEITEMCOMMAND_H
#define PURCHASEITEMCOMMAND_H

#include "command/Command.h"

class PurchaseItemCommand : public Command {
private:
    std::string logMessage;

public:
    void execute() override;
    std::string getLog() const override;
};

#endif
