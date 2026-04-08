#include "command/RefundCommand.h"

void RefundCommand::execute() {
    logMessage = "Refund command executed";
}

std::string RefundCommand::getLog() const {
    return logMessage;
}
