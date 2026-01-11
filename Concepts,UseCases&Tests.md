# Concepts&UseCases

---

## Concepts

1. TCP/IP  and Socket Programming
2. Concurrency and Multithreading
3. Mutex and Sync..
4. Data Structures -> Hash Tables, Vectors
5. Parsing and RESP protocol 
6. File I/O Presitance
7. Signal Handling
8. Command Processing and Response Formatting 
9. Singleton Pattern 
10. Bitwise Opretors '|=' 
11. std libraries

### Classes

- RedisServer class 

- RedisDatabase class

- RedisCommandHandler class

---

## Commands & Operations

---

### Common Commands

- **PING**  
  *Use case:* Before embarking on any data operations, an application can send a `PING` to ensure that the Redis server is alive and responsive—like knocking on a door before entering.

- **ECHO**  
  *Use case:* A debugging tool or simple utility to test network connectivity by having the server repeat a message. It can also be used in logging systems to trace commands.

- **FLUSHALL**  
  *Use case:* When resetting a cache or starting fresh, `FLUSHALL` clears all stored keys. This is useful during development or when you need to wipe out stale data completely.

---

### Key/Value Operations

- **SET**  
  *Use case:* Caching user session information or configuration settings. For example, store a user token with `SET session:123 "user_data"`.

- **GET**  
  *Use case:* Retrieve stored configuration or session data. For instance, fetch the user session with `GET session:123`.

- **KEYS**  
  *Use case:* List all keys or keys matching a pattern (e.g., `KEYS session:*`) to analyze cache usage or to perform maintenance tasks.

- **TYPE**  
  *Use case:* Check what type of value is stored at a key—useful when the data structure can vary, such as determining if a key holds a string, list, or hash.

- **DEL / UNLINK**  
  *Use case:* Remove keys that are no longer valid. This might be used to evict a stale cache entry after a user logs out or when cleaning up expired data.

- **EXPIRE**  
  *Use case:* Set a timeout on keys for caching. For example, cache product details for 3600 seconds so that the cache automatically evicts old data.

- **RENAME**  
  *Use case:* When restructuring keys during a migration or data reorganization, use `RENAME` to change the key’s name without losing its data.

---

### List Operations

- **LGET**  
  *Use case:* Returns all elements of a list at a given key (much like LRANGE key 0 -1 in real Redis).

- **LLEN**  
  *Use case:* Check the number of items in a message queue. For instance, determine how many tasks are pending in a job queue.

- **LPUSH / RPUSH**  
  *Use case:* Add items to a list. This is common in task queues or message brokers—`LPUSH` can be used to add a high-priority task at the beginning, while `RPUSH` appends regular tasks at the end.

- **LPOP / RPOP**  
  *Use case:* Remove items from a list. For example, dequeue a task from the beginning with `LPOP` or remove the last entry with `RPOP` in a double-ended queue.

- **LREM**  
  *Use case:* Remove specific elements from a list. For instance, eliminate duplicate notifications or remove a cancelled task from a list.

- **LINDEX**  
  *Use case:* Retrieve an element at a specific index. Useful when you need to inspect a particular item in a task queue without removing it.

- **LSET**  
  *Use case:* Update an element at a given position. This might be used in a real-time messaging app where you need to modify a message that is stored in a list.

---

### Hash Operations

- **HSET**  
  *Use case:* Store multiple fields for an object. For example, a user profile with `HSET user:1000 name "Alice" age "30" email "alice@example.com"`.

- **HGET**  
  *Use case:* Retrieve a specific field from a hash. For instance, fetching the email of a user with `HGET user:1000 email`.

- **HEXISTS**  
  *Use case:* Check if a particular field exists in a hash. For example, verify if a user profile has an "address" field.

- **HDEL**  
  *Use case:* Remove a field from a hash. Use it to delete outdated information, like removing a phone number when a user updates their profile.

- **HGETALL**  
  *Use case:* Retrieve all fields and values of a hash. This is useful when you need the full data set of an object, such as fetching an entire user profile.

- **HKEYS / HVALS**  
  *Use case:* List all fields (HKEYS) or all values (HVALS) in a hash, which is useful for displaying summary information or iterating through all data points in an object.

- **HLEN**  
  *Use case:* Determine the number of fields in a hash. For example, quickly checking how many attributes are stored in a user profile.

- **HMSET** 
  *Use case:* Set multiple fields in a hash at once. This can be useful for batch updates or initializing an object with several properties simultaneously.

---

## Tests:

---

### 1. Start Your Custom Server

```bash
./your_project_executable 6379
```

---

### 2. Connect with redis‑cli

```bash
redis-cli -p 6379
```

---

### 3. Common Commands

| Command        | Example                             | Expected Reply  |
| -------------- | ----------------------------------- | --------------- |
| **PING**       | `PING`                              | `PONG`          |
| **ECHO**       | `ECHO "Hello World"`                | `Hello World`   |
| **FLUSHALL**   | `FLUSHALL`                          | `OK`            |

---

### 4. Key/Value Operations

| Command               | Example                                  | Expected Reply                    |
| --------------------- | ---------------------------------------- | --------------------------------- |
| **SET** / **GET**     | `SET mykey "myvalue"`<br>`GET mykey`     | `OK`<br>`"myvalue"`               |
| **KEYS**              | `KEYS *`                                 | List of keys                      |
| **TYPE**              | `TYPE mykey`                             | `string`                          |
| **DEL** / **UNLINK**  | `DEL mykey`                              | `(integer) 1`                     |
| **EXPIRE**            | `SET session:1 "data"`<br>`EXPIRE session:1 5` | `OK`                          |
| **RENAME**            | `SET a "x"`<br>`RENAME a b`<br>`GET b`   | `OK`<br>`"x"`                     |

> **TIP:** After `EXPIRE`, do any operation (e.g. `GET`) after the TTL to see that the key is gone.

---

### 5. List Operations

| Command            | Example                                                           | Expected Reply                    |
| ------------------ | ----------------------------------------------------------------- | --------------------------------- |
| **LGET**           | `RPUSH L a b c`<br>`LGET L`                                       | `1) "a"`<br>`2) "b"`<br>`3) "c"`   |
| **LLEN**           | `LLEN L`                                                          | `(integer) 3`                     |
| **LPUSH / RPUSH**  | `LPUSH L "start"`<br>`RPUSH L "end"`                              | `(integer) 4`<br>`(integer) 5`    |
| **LPOP / RPOP**    | `LPOP L`<br>`RPOP L`                                              | `"start"`<br>`"end"`              |
| **LREM**           | `RPUSH L x y x z x`<br>`LREM L 2 x`<br>`LREM L 0 x`               | `(integer) 2`<br>`(integer) <n>`  |
| **LINDEX**         | `LINDEX L 1`<br>`LINDEX L -1`                                     | `"y"`<br>`"z"`                    |
| **LSET**           | `LSET L 1 "new_val"`<br>`LINDEX L 1`                              | `OK`<br>`"new_val"`               |

---

### 6. Hash Operations

| Command           | Example                                                                                                 | Expected Reply                                                 |
| ----------------- | ------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| **HSET**          | `HSET user:1 name Alice age 30 email a@x.com`                                                           | `(integer) 1` (per field)                                      |
| **HGET**          | `HGET user:1 email`                                                                                     | `"a@x.com"`                                                    |
| **HEXISTS**       | `HEXISTS user:1 address`<br>`HEXISTS user:1 name`                                                        | `(integer) 0`<br>`(integer) 1`                                 |
| **HDEL**          | `HDEL user:1 age`<br>`HEXISTS user:1 age`                                                               | `(integer) 1`<br>`(integer) 0`                                 |
| **HLEN**          | `HLEN user:1`                                                                                            | `(integer) 2`                                                  |
| **HKEYS**         | `HKEYS user:1`                                                                                           | `1) "name"`<br>`2) "email"`                                    |
| **HVALS**         | `HVALS user:1`                                                                                           | `1) "Alice"`<br>`2) "a@x.com"`                                 |
| **HGETALL**       | `HGETALL user:1`                                                                                         | `1) "name"`<br>`2) "Alice"`<br>`3) "email"`<br>`4) "a@x.com"`  |
| **HMSET**         | `HMSET user:2 name Bob age 25 city Paris`<br>`HGETALL user:2`                                           | `OK`<br>`1) "name"`<br>`2) "Bob"`<br>`3) "age"`<br>`4) "25"`<br>`5) "city"`<br>`6) "Paris"` |

---