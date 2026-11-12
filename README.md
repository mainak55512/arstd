# `arstd` — Arena Standard Library for C

**`arstd` (Arena Standard Library)** is a lightweight, high-performance foundational toolkit for C that eliminates manual `malloc`/`free` bookkeeping by uniting memory management and core data structures under a shared **Arena Allocator**. 

Through a single umbrella header (`#include <arstd.h>`), `arstd` provides region-based memory allocation (`Arena`), compile-time type-safe segmented dynamic arrays (`Vector(T)`), single-allocation generic hash maps (`Cmap(T)`), and managed strings (`String`). All containers are engineered to allocate contiguously, grow with zero wasted arena space, and reclaim all memory in `O(1)` time with a single `arena_free(&arena)` call—while retaining an aligned heap fallback when `NULL` is passed instead of an arena.

---

## Key Features

* **Unified Arena Lifecycle (`arena.h`):** Allocate memory for vectors, hash maps, and strings from a single `Arena *`. Reset or free the entire object graph in one call without walking linked lists or freeing individual buffers.
* **Compile-Time Type Deduction (`typeof`):** Declare typed containers (`Vector(int)`, `Cmap(String *)`) that deduce element and value types automatically—no `void *` casts or passing `TYPE` to every macro call.
* **Zero-Copy Segmented Vectors (`cvector.h`):** Dynamic arrays backed by exponentially growing segments ($16, 32, 64\dots$). Resizing never calls `realloc` or `memcpy`, leaving zero dead space in the arena and keeping element pointers stable. Indexing via `at()` runs in $O(1)$ branchless time using bitwise `CLZ` math.
* **Single-Allocation Type-Safe Hash Maps (`cmap.h`):** Generic separate-chaining hash map that packs the entry header, aligned value payload, and key string into a single arena allocation (`char data[]`). Features cached 64-bit hashes, bitwise bucket masks, zero-waste resizing, and $O(1)$ `map_keys()` / `map_remove()`.
* **Managed Strings (`cstring.h`):** Safe, arena-backed string creation, slicing, trimming, case conversion, variadic concatenation, and comparison utilities.

---

## Installation & Build Steps

### Using [flint](https://github.com/mainak55512/flint)

Add `arstd` directly to your `flint` project:

```bash
flint add https://github.com/mainak55512/arstd@v0.1.1
```

### Using CMake

1. Clone the repository and navigate to the project directory:
   ```bash
   git clone https://github.com/mainak55512/arstd.git
   cd arstd
   ```
2. Create a build directory and compile:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

This produces the static library `libarstd.a`. Include the entire toolkit in your code with:

```c
#include <arstd.h>
```
*(Or include individual modules selectively via `<arena.h>`, `<cvector.h>`, `<cmap.h>`, and `<cstring.h>`).*

---

## API Reference

### 1. Arena & Aligned Memory (`<arena.h>`)

| Function / Macro | Description | Signature |
| --- | --- | --- |
| `arena_init` | Initializes a new `Arena` with the specified initial byte capacity (rounded up to `MAX_ALIGN`). | `Arena *arena_init(size_t capacity)` |
| `arena_alloc` | Allocates a zero-initialized, maximally aligned chunk of `size` bytes from the arena (automatically chains a new block if full). | `void *arena_alloc(Arena *arena, size_t size)` |
| `arena_push` | Typed helper macro that allocates `count` zero-initialized elements of type `T` on the arena. | `T *arena_push(Arena *arena, T, size_t count)` |
| `arena_reset` | Wipes all allocations, frees any chained overflow blocks, and restores the arena to its initial base capacity. | `void arena_reset(Arena **arena)` |
| `arena_rewind` | Zeroes out all used bytes and resets offsets to `0` across all chained blocks without freeing them, keeping capacity warm for reuse. | `void arena_rewind(Arena *arena)` |
| `arena_free` | Securely zeroes out and frees the arena and all chained memory blocks, setting `*arena` to `NULL`. | `void arena_free(Arena **arena)` |
| `arena_temp_begin` | Takes an $O(1)$ snapshot of the arena's current position for temporary scratchpad allocations. | `ArenaTemp arena_temp_begin(Arena *arena)` |
| `arena_temp_end` | Rewinds and zeroes out all allocations made since `arena_temp_begin`, restoring the arena to the saved checkpoint. | `void arena_temp_end(ArenaTemp temp)` |
| `arena_used` | Returns the total number of bytes currently allocated across all chained blocks in the arena. | `size_t arena_used(Arena *arena)` |
| `arena_capacity` | Returns the total byte capacity allocated across all chained blocks in the arena. | `size_t arena_capacity(Arena *arena)` |
| `m_align_alloc` | Allocates zero-initialized heap memory aligned to the platform's maximum alignment requirement (`MAX_ALIGN`). | `void *m_align_alloc(size_t capacity)` |
| `m_align_realloc` | Reallocates maximally aligned heap memory from `old_size` to `new_size`. | `void *m_align_realloc(void *ptr, size_t old_size, size_t new_size)` |
| `m_align_free` | Frees memory allocated by `m_align_alloc` or `m_align_realloc`. | `void m_align_free(void *ptr)` |

---

### 2. Segmented Dynamic Array (`<cvector.h>`)

| Macro / Function | Description | Signature (as seen by user) |
| :--- | :--- | :--- |
| `Vector(TYPE)` | Declares a typed vector handle for compile-time type checking and deduction. | `Vector(TYPE) vec` |
| `vector_init` | Initializes a new segmented vector backed by `arena` (or `NULL` for heap allocation). | `Vector(TYPE) vector_init(Arena *arena)` |
| `append` | Appends a copy of `value` to the end of `vec`. | `void append(Vector(TYPE) vec, TYPE value)` |
| `at` | Accesses the element at `pos` in $O(1)$ time. Returns a **modifiable lvalue** (`at(vec, 0) = val`). | `TYPE at(Vector(TYPE) vec, int pos)` |
| `pop` | Removes and returns the last element of `vec`. | `TYPE pop(Vector(TYPE) vec)` |
| `length` | Returns the current number of elements in `vec`. | `int length(Vector(TYPE) vec)` |
| `vector_free` | Frees heap memory if initialized with `NULL` (safe pointer-nulling no-op if arena-backed). | `void vector_free(Vector(TYPE) *vec_ptr)` |

---

### 3. Type-Safe Hash Map (`<cmap.h>`)

| Macro / Function | Description | Signature (as seen by user) |
| :--- | :--- | :--- |
| `Cmap(TYPE)` | Declares a typed hash map handle with `const char *` keys and values of `TYPE`. | `Cmap(TYPE) map` |
| `map_init` | Allocates and initializes a new hash map on `arena` (or `NULL` for heap allocation). | `Cmap(TYPE) map_init(Arena *arena)` |
| `map_add` | Inserts or updates `key` by copying `val` directly into the map entry. | `void map_add(Cmap(TYPE) map, const char *key, TYPE val)` |
| `map_get` | Looks up `key` and returns a **modifiable lvalue** of type `TYPE`. | `TYPE map_get(Cmap(TYPE) map, const char *key)` |
| `map_has` | Returns `true` if `key` exists in the map, or `false` otherwise. | `bool map_has(Cmap(TYPE) map, const char *key)` |
| `map_remove` | Removes `key` from the map and updates the key list in $O(1)$ time. Returns `true` if removed. | `bool map_remove(Cmap(TYPE) map, const char *key)` |
| `map_keys` | Returns the internal `Vector(char *)` containing all active keys in $O(1)$ time. | `Vector(char *) map_keys(Cmap(TYPE) map)` |
| `map_len` | Returns the total number of active key-value pairs in $O(1)$ time. | `size_t map_len(Cmap(TYPE) map)` |
| `map_free` | Frees heap memory if initialized with `NULL` (safe pointer-nulling no-op if arena-backed). | `void map_free(Cmap(TYPE) *map_ptr)` |

---

### 4. Managed String Library (`<cstring.h>`)

| Function | Description | Signature |
| :--- | :--- | :--- |
| `string_from` | Creates a `String *` object from a null-terminated C-string (`char *`). | `String *string_from(Arena *arena, char *str)` |
| `string_from_len` | Creates a `String *` object from a C-string with an explicit `len`. | `String *string_from_len(Arena *arena, size_t len, char *str)` |
| `string_clone` | Creates a deep copy of an existing `String *` object on `arena`. | `String *string_clone(Arena *arena, String *str)` |
| `string_get` | Reads a line of input from the console (`stdin`) into a new `String *`. | `String *string_get(Arena *arena)` |
| `string` | Returns the underlying null-terminated `char *` buffer from a `String *`. | `char *string(String *st)` |
| `string_len` | Returns the character length of `str`. | `int string_len(String *str)` |
| `string_sub` | Returns a substring from `begin` (inclusive) to `end` (exclusive). | `String *string_sub(Arena *arena, String *str, int begin, int end)` |
| `string_concat` | Concatenates `n` `String *` objects into a new `String *`. | `String *string_concat(Arena *arena, int n, ...)` |
| `string_concat_cstr` | Concatenates `n` raw C-strings (`char *`) into a new `String *`. | `String *string_concat_cstr(Arena *arena, int n, ...)` |
| `string_trim` | Returns a new `String *` with all leading and trailing whitespace removed. | `String *string_trim(Arena *arena, String *str)` |
| `string_upper` | Returns a new `char *` with all characters converted to uppercase. | `char *string_upper(Arena *arena, String *str)` |
| `string_lower` | Returns a new `char *` with all characters converted to lowercase. | `char *string_lower(Arena *arena, String *str)` |
| `string_comp` | Case-insensitive comparison of two `String *` objects (`true` if equal). | `bool string_comp(struct String *s1, struct String *s2)` |
| `string_comp_case` | Case-sensitive comparison of two `String *` objects (`true` if equal). | `bool string_comp_case(struct String *s1, struct String *s2)` |

---

## Unified Example

The example below demonstrates how `Arena`, `Vector(T)`, `Cmap(T)`, and `String` work together seamlessly without a single manual `free()` call for individual containers or strings:

```c
#include <arstd.h>
#include <stdio.h>

int main(void) {
    // 1. Initialize a single shared Arena for the entire program/task
    Arena *arena = arena_init(4096);

    // 2. Create and manipulate managed Strings on the arena
    String *raw_title = string_from(arena, "   arstd: arena standard library   ");
    String *trimmed   = string_trim(arena, raw_title);
    char   *banner    = string_upper(arena, trimmed);
    printf("=== %s (len: %d) ===\n\n", banner, string_len(trimmed));

    // 3. Create a typed segmented Vector of integers
    Vector(int) primes = vector_init(arena);
    append(primes, 2);
    append(primes, 3);
    append(primes, 5);
    at(primes, 2) = 7; // Direct lvalue modification via at()

    printf("Primes (%d items): ", length(primes));
    for (int i = 0; i < length(primes); i++) {
        printf("%d ", at(primes, i));
    }
    printf("\n\n");

    // 4. Create a typed Hash Map mapping usernames (const char *) to String* objects
    Cmap(String *) profiles = map_init(arena);

    map_add(profiles, "alice", string_concat_cstr(arena, 2, "Role: ", "Architect"));
    map_add(profiles, "bob",   string_concat_cstr(arena, 2, "Role: ", "Systems Engineer"));
    map_add(profiles, "charlie", string_from(arena, "Role: Maintainer"));

    // Mutate an entry in-place and remove another
    map_get(profiles, "bob") = string_from(arena, "Role: Lead Systems Engineer");
    map_remove(profiles, "charlie");

    // 5. Iterate over active map keys in O(1) time
    Vector(char *) keys = map_keys(profiles);
    printf("Active Profiles (%zu):\n", map_len(profiles));
    for (int i = 0; i < length(keys); i++) {
        char *user = at(keys, i);
        String *info = map_get(profiles, user);
        printf("  - %-8s -> %s\n", user, string(info));
    }

    // 6. Reclaim ALL memory (Strings, Vectors, Hash Map buckets, and Entries) in one call!
    arena_free(&arena);
    return 0;
}
```
