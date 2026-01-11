# my_redis_server

A small, Redis-compatible in-memory data store I built from scratch in C++17 to learn how Redis actually works under the hood.

No third-party libraries, no `hiredis`, no `libuv`. Just the C++ standard library, POSIX sockets, and a lot of staring at Wireshark to understand RESP. The goal was never to replace Redis — it was to write every layer myself until none of it felt like magic.

It speaks the real [RESP protocol](https://redis.io/docs/reference/protocol-spec/) over a plain TCP socket, so you can point the official `redis-cli` at it and it just works:

```bash
$ ./my_redis_server 6379
$ redis-cli -p 6379 PING
PONG
$ redis-cli -p 6379 SET hello world
OK
$ redis-cli -p 6379 GET hello
"world"
```

---

## Why I built this

I'd been using Redis for years as a black box and I wanted to finally open it. In particular I wanted to get hands-on with:

- **TCP / socket programming** in C — what `accept()`, `recv()`, `send()` actually do.
- **A real wire protocol** — RESP is simple enough to implement in an afternoon, but hits all the interesting parts (length-prefixed bulk strings, arrays, error responses).
- **Multi-threaded concurrency in C++** — `std::thread`, `std::mutex`, `std::atomic`, and the sharp edges that come with them.
- **Lazy expiration** and why real databases don't just run a timer per key.
- **Persistence trade-offs** — snapshotting vs. append-only log, and why my naïve dump format breaks on spaces.
- **Signal handling** — graceful shutdown on `Ctrl+C`.
- **C++ idioms I'd never had a reason to reach for** — singletons done right (Meyers singleton / magic statics), RAII locks, function-local `static` with thread-safe init.

If you're learning the same things, I hope reading this is useful.

---

## What it does

- Listens on TCP port `6379` (or any port you pass on the command line).
- Parses RESP from the wire — both the array form that `redis-cli` sends and the inline "telnet" form.
- Supports three data types backed by `std::unordered_map`:
  - **Strings** — `kv_store: unordered_map<string, string>`
  - **Lists** — `list_store: unordered_map<string, vector<string>>`
  - **Hashes** — `hash_store: unordered_map<string, unordered_map<string, string>>`
- Handles multiple clients concurrently with one `std::thread` per connection, all serialized through a single `std::mutex` on the database.
- Lazy TTL eviction via `std::chrono::steady_clock` — expired keys are purged the next time anyone touches them.
- Snapshots to a text file (`dump.my_rdb`) every 5 minutes and on `Ctrl+C`, and reloads it on startup.

---

## Project layout

```
.
├── include/
│   ├── RedisCommandHandler.h   # RESP parser + dispatch
│   ├── RedisDatabase.h         # Singleton in-memory store
│   └── RedisServer.h           # TCP listener + signal handling
├── src/
│   ├── RedisCommandHandler.cpp
│   ├── RedisDatabase.cpp
│   ├── RedisServer.cpp
│   └── main.cpp                # Entry point, loads dump, starts bg thread
├── NOTES.md                    # Design notes, per-command use cases, tests
├── Makefile                    # make, make clean, make rebuild, make run
├── test_all.sh                 # End-to-end smoke test via redis-cli
└── README.md
```

---

## Build

Needs a C++17 compiler and `make`. Tested on macOS (Apple clang) and Linux (g++).

```bash
make            # produces ./my_redis_server
make clean      # nukes build/ and the binary
make rebuild    # clean + build
```

Or compile by hand:

```bash
g++ -std=c++17 -pthread -Wall -O2 -Iinclude src/*.cpp -o my_redis_server
```

---

## Run

```bash
./my_redis_server            # listens on 6379
./my_redis_server 6380       # custom port
```

First-run output:

```
No dump found or load failed; starting with an empty database.
Redis Server Listening On Port 6379
```

A background thread snapshots the database every 5 minutes. `Ctrl+C` triggers a graceful shutdown that dumps one more time and closes the listening socket.

---

## Try it with redis-cli

Install `redis-cli` only (you don't need a Redis server — this *is* the server):

```bash
brew install redis              # macOS
sudo apt install redis-tools    # Debian/Ubuntu
```

Then in another terminal:

```bash
$ redis-cli -p 6379
127.0.0.1:6379> PING
PONG
127.0.0.1:6379> SET foo "bar"
OK
127.0.0.1:6379> GET foo
"bar"
127.0.0.1:6379> RPUSH queue task1 task2 task3
(integer) 3
127.0.0.1:6379> LGET queue
1) "task1"
2) "task2"
3) "task3"
127.0.0.1:6379> HMSET user:1 name Alice age 30 email alice@example.com
OK
127.0.0.1:6379> HGETALL user:1
1) "name"
2) "Alice"
3) "age"
4) "30"
5) "email"
6) "alice@example.com"
```

Or run the scripted sweep (server must be running first):

```bash
chmod +x test_all.sh
./test_all.sh
```

---

## Supported commands

| Category | Commands |
|---|---|
| **Connection** | `PING`, `ECHO`, `FLUSHALL` |
| **Keys** | `SET`, `GET`, `KEYS`, `TYPE`, `DEL` / `UNLINK`, `EXPIRE`, `RENAME` |
| **Lists** | `LGET`, `LLEN`, `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LREM`, `LINDEX`, `LSET` |
| **Hashes** | `HSET`, `HGET`, `HEXISTS`, `HDEL`, `HKEYS`, `HVALS`, `HLEN`, `HGETALL`, `HMSET` |

See [`NOTES.md`](NOTES.md) for semantics, use cases, and an expected-reply table for every command.

---

## Architecture at a glance

```
┌────────────┐   TCP (RESP)   ┌────────────────────────┐
│ redis-cli  │──────────────▶│  RedisServer           │
└────────────┘                │  accept loop, 1 thread │
                              │  per connection        │
                              └──────────┬─────────────┘
                                         │
                                         ▼
                              ┌────────────────────────┐
                              │  RedisCommandHandler   │
                              │  parseRespCommand()    │
                              │  dispatch              │
                              └──────────┬─────────────┘
                                         │
                                         ▼
                              ┌────────────────────────┐
                              │  RedisDatabase         │
                              │  (singleton + mutex)   │
                              │  kv / list / hash maps │
                              │  + expiry_map          │
                              └──────────┬─────────────┘
                                         │ every 300 s and on SIGINT
                                         ▼
                                   dump.my_rdb
```

Three classes. One singleton. One mutex. One thread per client. One background persistence thread. That's the whole thing.

---

## What I learned

- **RESP is tiny but strict.** Every byte is load-bearing — off-by-one on a `\r\n` and the client just hangs waiting for more bytes. Writing the parser by hand was the single biggest win.
- **A single coarse-grained mutex is fine until it isn't.** For a personal-scale server it's provably correct and readable. I know the scaling path (sharded locks, or an event loop instead of threads) but coarse-first was the right call for a first pass.
- **Lazy eviction is elegant.** No timer thread, zero idle cost, and memory is only reclaimed when someone's already paying the latency cost of a lookup. Real Redis adds a background sampler on top; I skipped that.
- **Singletons via function-local `static` are thread-safe in C++11+**. I used to implement them with `std::call_once` and mutexes. "Magic statics" are simpler and generate the same code.
- **`std::atomic<bool>` isn't a gimmick.** The accept loop reads `running`; the signal handler writes it. Without `std::atomic` that's UB, and it will bite you.
- **Text-based snapshots are a trap.** My dump format uses spaces and colons as separators, so the moment a key contains either one the round-trip silently corrupts data. A proper binary format with length prefixes (basically RESP on disk) would be the honest fix.

---

## Known limitations / things I'd do next

- [ ] Persistence format is whitespace-delimited and breaks on keys/values containing spaces or `:`. Switch to a length-prefixed binary format.
- [ ] Thread-per-connection doesn't scale past ~hundreds of clients. Port to `epoll` / `kqueue` with a single-threaded event loop.
- [ ] One global mutex = all operations serialize. Shard the lock per key bucket, or per data type, once contention is measurable.
- [ ] No pub/sub, no transactions (`MULTI` / `EXEC`), no Lua scripting, no replication. All are interesting extensions.
- [ ] RESP3 support — currently only RESP2.
- [ ] Real configurable snapshot interval and an AOF-style append-only log option.

---

## License

MIT. See `LICENSE` if/when I get around to adding one.
