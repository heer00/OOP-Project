#include "command/PurchaseItemCommand.h"

void PurchaseItemCommand::execute() {
    logMessage = "Purchase command executed";
}

std::string PurchaseItemCommand::getLog() const {
    return logMessage;
}
