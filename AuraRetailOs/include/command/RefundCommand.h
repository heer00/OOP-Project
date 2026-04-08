#ifndef REFUNDCOMMAND_H
#define REFUNDCOMMAND_H

#include "command/Command.h"

class RefundCommand : public Command {
private:
    std::string logMessage;

public:
    void execute() override;
    std::string getLog() const override;
};

#endif
