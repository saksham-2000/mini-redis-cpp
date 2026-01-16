#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * RedisDatabase
 *
 * The in-memory store behind the whole server. Three backing maps (strings,
 * lists, hashes) plus a TTL deadline map, all guarded by a single mutex.
 *
 * This is intentionally a Meyers singleton: exactly one instance per process,
 * lazily constructed on first use, thread-safe for free since C++11
 * guarantees function-local statics are initialized exactly once.
 *
 * Expiration is lazy: we never run a timer. Every read-ish method calls
 * purgeExpired() under the lock, which sweeps the deadline map and evicts
 * anything whose time has come. Costs a tiny bit on hot paths; costs nothing
 * when the server is idle.
 */
class RedisDatabase {
public:
    static RedisDatabase& getInstance();

    // ---- Connection/server-level ------------------------------------------------
    bool flushAll();

    // ---- String (key/value) -----------------------------------------------------
    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    std::vector<std::string> keys();
    std::string type(const std::string& key);
    bool del(const std::string& key);
    bool expire(const std::string& key, int seconds);
    bool rename(const std::string& oldKey, const std::string& newKey);

    // ---- Lists ------------------------------------------------------------------
    std::vector<std::string> lget(const std::string& key);
    ssize_t llen(const std::string& key);
    void lpush(const std::string& key, const std::string& value);
    void rpush(const std::string& key, const std::string& value);
    bool lpop(const std::string& key, std::string& value);
    bool rpop(const std::string& key, std::string& value);
    int lrem(const std::string& key, int count, const std::string& value);
    bool lindex(const std::string& key, int index, std::string& value);
    bool lset(const std::string& key, int index, const std::string& value);

    // ---- Hashes -----------------------------------------------------------------
    // Returns true if the field was newly added, false if it already existed
    // and was overwritten. Callers use this to count "new fields" for HSET.
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    bool hget(const std::string& key, const std::string& field, std::string& value);
    bool hexists(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::unordered_map<std::string, std::string> hgetall(const std::string& key);
    std::vector<std::string> hkeys(const std::string& key);
    std::vector<std::string> hvals(const std::string& key);
    ssize_t hlen(const std::string& key);
    bool hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fieldValues);

    // ---- Persistence ------------------------------------------------------------
    // Text-format snapshot. Good enough for a learning project; it will corrupt
    // keys/values that contain spaces or colons. Fixing that is on the roadmap.
    bool dump(const std::string& filename);
    bool load(const std::string& filename);

private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    // Must be called while `db_mutex` is held. Evicts every key whose deadline
    // has passed. This is what makes expiration "lazy".
    void purgeExpired();

    std::mutex db_mutex;
    std::unordered_map<std::string, std::string> kv_store;
    std::unordered_map<std::string, std::vector<std::string>> list_store;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_map;
};

#endif
