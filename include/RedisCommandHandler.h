#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <string>

class RedisCommandHandler {
public:
    RedisCommandHandler();
    // Process a command from a client and return a RESP-formatted response.
    std::string processCommand(const std::string& commandLine);
};

#endif
