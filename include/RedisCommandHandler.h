#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class RedisDatabase;

class RedisCommandHandler {
public:
    RedisCommandHandler();

    // Parse one RESP-framed request, dispatch it, and return the RESP reply.
    std::string processCommand(const std::string& commandLine);

private:
    // A handler takes the already-tokenized command and the shared database,
    // and returns a fully-formed RESP reply string.
    using Handler = std::function<std::string(const std::vector<std::string>&, RedisDatabase&)>;

    // Built once in the constructor; keyed by the upper-cased command name.
    std::unordered_map<std::string, Handler> dispatch_table;
};

#endif
