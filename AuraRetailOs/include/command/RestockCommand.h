#ifndef RESTOCKCOMMAND_H
#define RESTOCKCOMMAND_H

#include "command/Command.h"

class RestockCommand : public Command {
private:
    std::string logMessage;

public:
    void execute() override;
    std::string getLog() const override;
};

#endif
