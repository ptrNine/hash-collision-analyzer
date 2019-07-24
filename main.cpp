#include "3rd/libcuckoo/libcuckoo/cuckoohash_map.hh"
#include "collision_test.hpp"


// You can easily add you own hash functions
//
// There are two requirements:
// 1. Hash function must accept const uint8_t* array and size_t length
// 2. Hash function must return integers or type with specialized 'operator<' and 'ostream operator<<'


uint32_t djb(const uint8_t* ptr, size_t size) {
    uint32_t hash = 5381;

    while (size--)
        hash = (hash << 5U) + hash + *ptr++;

    return hash;
}

uint32_t fnv1a32(const uint8_t* ptr, size_t size) {
    uint32_t hash = 0x811c9dc5;

    while (size--)
        hash = (hash ^ *ptr++) * 0x01000193;

    return hash;
}

uint64_t fnv1a64(const uint8_t* ptr, size_t size) {
    uint64_t hash = 0xcbf29ce484222325;

    while (size--)
        hash = (hash ^ *ptr++) * 0x100000001b3;

    return hash;
}


#include "md5.hpp"

static auto hash_functions = std::make_tuple(
        hctest::hf(djb,      "djb",     "DJB"        ),
        hctest::hf(fnv1a32,  "fnv1a32", "FNV1a 32bit"),
        hctest::hf(fnv1a64,  "fnv1a64", "FNV1a 64bit"),
        hctest::hf(md5::md5, "md5",     "MD5"        )
);


int main(int argc, char* argv[]) {
    auto hct = hctest::HashCollisionTest(hash_functions);

    return hct.run(argc, argv);
}