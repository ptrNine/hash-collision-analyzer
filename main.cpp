#include "3rd/libcuckoo/libcuckoo/cuckoohash_map.hh"
#include "collision_test.hpp"


// You can easily add you own hash functions
//
// There are two requirements:
// 1. Hash function must accept ONLY const char* string with null terminator
// 2. Hash function must return integers or type with specialized 'operator<' and 'ostream operator<<'


uint32_t djb(const char* ptr) {
    if (!ptr)
        return 0;

    uint32_t hash = 5381;

    while (*ptr)
        hash = (hash << 5U) + hash + *ptr++;

    return hash;
}

uint32_t fnv1a32(const char* ptr) {
    if (!ptr)
        return 0;

    uint32_t hash = 0x811c9dc5;

    while (*ptr) {
        hash ^= *ptr++;
        hash *= 0x01000193;
    }

    return hash;
}

uint64_t fnv1a64(const char* ptr) {
    if (!ptr)
        return 0;

    uint64_t hash = 0xcbf29ce484222325;

    while (*ptr) {
        hash ^= *ptr++;
        hash *= 0x100000001b3;
    }

    return hash;
}


#include "md5.hpp"

static auto hash_functions = hctest::generate_hct_tuple(
    HF (djb),
    HF (fnv1a32),
    HF (fnv1a64),

    HFN(md5::md5, "md5")
);


int main(int argc, char* argv[]) {
    auto hct = hctest::HashCollisionTest(hash_functions);

    return hct.run(argc, argv);
}