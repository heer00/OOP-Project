#ifndef COMMAND_H
#define COMMAND_H

#include <string>

class Command {
public:
    virtual void execute() = 0;
    virtual std::string getLog() const = 0;
    virtual ~Command() {}
};

#endif
