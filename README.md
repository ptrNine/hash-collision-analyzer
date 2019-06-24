# hash-collision-analyzer
hash-collision-analyzer is tool for finding hash collisions from files with '\n' separated string sequence.
# Usage
Common usage:

    ./hash_collision [options]... [hash function name] [path/to/file]
    
For example:

    ./hash_collision djb strings.txt
`djb` - hash function name. `strings.txt` - file with `'\n'` separated strings.

Calling without parameters show list of available hash functions.

# Building
hash-collision-analyzer tested on Linux Mint 19 with g++ 8.2.0 and clang-7. Required C++17 for building.
Also hash-collision-analyzer depends on [libcuckoo](https://github.com/efficient/libcuckoo)
(used for finding hash collisions in multiple threads).

# Hash functions
Almost every hash function can be easily added.

There are two requirements:
1. Hash function must accept `const char*` string ONLY.
2. Hash function must return integers or type with specialized `operator<` and `ostream operator<<`

For example, this function:

```c++
uint32_t my_hash_func_impl(const char* str) {
    ...
}
```

can be easily added with:

```c++
static auto hash_functions = hctest::generate_hct_tuple(
    HF(djb),
    HF(fnv1a32),
    HF(fnv1a64),
    HF(my_hash_func_impl)
);
```

If hash function calculate checksums with size larger then 8 bytes then it should returns structure with defined
`ostream operator <<` and `operator <`:

```c++
class Checksum128 {
public:
    friend std::ostream& operator<<(std::ostream& os, const Checksum128& cs) {
        ...
        return os;
    }
    
    bool operator<(const Checksum128& cs) const {
        ...
    }
    
    uint32_t data[4];
}

Checksum128 my_hash_func_impl(const char* str) {
    ...
}
```
