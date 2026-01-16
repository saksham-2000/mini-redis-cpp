#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class RedisDatabase;

/**
 * RedisCommandHandler
 *
 * Turns a raw byte buffer from the wire into a parsed RESP array, routes
 * the command to its handler via an O(1) dispatch table, and returns a
 * RESP-formatted reply string. Stateless across calls; safe to share a
 * single instance across all client threads.
 *
 * Adding a new command is two steps:
 *   1. Write a `static std::string handleFoo(...)` at file scope.
 *   2. Add a `{"FOO", &handleFoo}` entry to dispatch_table.
 */
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
