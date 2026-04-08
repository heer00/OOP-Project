#include "command/RestockCommand.h"

void RestockCommand::execute() {
    logMessage = "Restock command executed";
}

std::string RestockCommand::getLog() const {
    return logMessage;
}
