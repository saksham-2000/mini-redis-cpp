#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include <atomic>
#include <string>

/**
 * RedisServer
 *
 * Owns the listening TCP socket and the accept() loop. Each accepted
 * connection is handed off to its own std::thread running a recv -> dispatch
 * -> send loop, so every client gets its own stack without blocking others.
 *
 * The class also wires up a SIGINT handler (via a global pointer - POSIX
 * signals are C-callable and can't capture state), which flips `running`
 * to false, persists the database, and closes the listening socket.
 */
class RedisServer {
public:
    explicit RedisServer(int port);

    // Blocks on accept() until shutdown() is called. Spawns one thread per
    // client and joins them all before returning.
    void run();

    // Idempotent. Safe to call from a signal handler.
    void shutdown();

private:
    int port;
    int server_socket;
    std::atomic<bool> running;

    // Signal handlers are C functions, so the "active server" has to live
    // in a translation-unit static in the .cpp. This just registers it.
    void setupSignalHandler();
};

#endif
