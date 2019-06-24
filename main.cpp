#include "3rd/libcuckoo/libcuckoo/cuckoohash_map.hh"
#include "collision_test.hpp"


////////////////// Hash functions ///////////////////////

uint32_t djb(const char* ptr) {
    if (!ptr)
        return 0;

    uint32_t hash = 5381;

    while (*ptr)
        hash = (hash << 5) + hash + *ptr++;

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


static auto hash_functions = hctest::generate_hct_tuple(
    HF(djb),
    HF(fnv1a32),
    HF(fnv1a64)
);




int main(int argc, char* argv[]) {
    auto hct = hctest::HashCollisionTest(hash_functions);


    cuckoohash_map<int, std::string> map;

    /*
    map.insert(1, "kek");
    map.find_fn(1, [](std::string& str) {
       str = "lel";
    });

    std::cout << map.find(1) << std::endl;
    //auto k = map.find(1, str);

    */

    return hct.run(argc, argv);
}



/*
auto process_buf(const char* start, const char* end) -> std::vector<std::string> {
    std::vector<std::string> strs;
    std::map<uint32_t, std::vector<std::string>> helpers;

    ptrdiff_t last_cve_index = -1;
    std::string last_cve_id = "CVE-12313123";

    auto not_exists = [&](const std::string& str) -> bool {
        if (str.find(last_cve_id, 0) == std::string::npos) {
            last_cve_id = str.substr(0, 8);
            last_cve_index = strs.size();
        }

        bool not_exist = true;
        auto hash  = djb(str.c_str());
        auto found = helpers.find(hash);

        if (found != helpers.end()) {
            for (auto& s : found->second) {
                if (str == s) {
                    not_exist = false;
                    break;
                }
            }
        }

        if (not_exist)
            helpers[hash].push_back(str);

        return not_exist;
    };

    std::string cve; cve.reserve(64);

    clock_t counter = clock();

    while (start < end) {
        cve.clear();

        while (*start != ' ')
            ++start;
        ++start;

        while(*start != '\n')
            cve.push_back(*start++);

        ++start;

        if (not_exists(cve))
            strs.push_back(cve);

        auto ccc = clock();
        if (ccc > counter + 1000000) {
            counter = ccc;
            std::cerr << std::this_thread::get_id() << ": " << cve << std::endl;
        }
    }

    return std::move(strs);
}
 */

/*
auto process_multithread(const std::vector<char>& buf, const std::array<size_t, 4>& data_steps) -> std::vector<std::string> {
    //auto f1 = std::async(std::launch::async, process_buf, buf.data()                , buf.data() + data_steps[0]);
    //auto f2 = std::async(std::launch::async, process_buf, buf.data() + data_steps[0], buf.data() + data_steps[1]);
    //auto f3 = std::async(std::launch::async, process_buf, buf.data() + data_steps[1], buf.data() + data_steps[2]);
    auto f4 = std::async(std::launch::async, process_buf, buf.data() + data_steps[2], buf.data() + data_steps[3]);

    //auto v1 = f1.get();
    //auto v2 = f2.get();
    //auto v3 = f3.get();
    auto v4 = f4.get();

    std::vector<std::string> res;

    //res.insert(res.end(), v1.begin(), v1.end());
    //res.insert(res.end(), v2.begin(), v2.end());
    //res.insert(res.end(), v3.begin(), v3.end());
    res.insert(res.end(), v4.begin(), v4.end());

    return std::move(res);
}

auto divide_data(const std::vector<char>& buf) -> std::array<size_t, 4> {
    const char* beg   = buf.data();
    const char* start = buf.data();
    const char* end   = buf.data() + buf.size();

    std::vector<size_t> positions;

    while (start < end) {
        positions.push_back(start - beg);
        while (*start++ != ' ');
        while (*start++ != '\n');
    }

    std::cerr << "Total lines: " << positions.size() << std::endl;

    return  {
                positions[ positions.size() / 4],
                positions[(positions.size() / 4) * 2],
                positions[(positions.size() / 4) * 3],
                positions.back()
            };
};
 */

/*
    auto buf = read_file_to_buffer(argv[1]);

    if (buf.empty())
        return -1;

    auto strs = process_buf(buf.data(), buf.data() + buf.size());//process_multithread(buf, divide_data(buf));

    for (auto& s : strs)
        std::cout << s << std::endl;


*/

    //auto strs = process_buf(buf.data(), buf.data() + buf.size());


    //std::map<int, std::vector<std::string>> hashes;

        /*

        auto found = hashes.find(hash);
        if (found == hashes.end())
            hashes[hash].push_back(str);
        else {
            bool isFnd = false;

            for (auto& s : found->second)
                if (s == str)
                    isFnd = true;

            if (!isFnd) {
                if (found->second.size() == 1)
                    total_count++;

                found->second.push_back(str);
            }
        }
         */
/*
    std::cout << "Total collisions count: " << total_count << std::endl;
    std::cout << std::endl;
    for (auto& kv : hashes) {
        if (kv.second.size() > 1) {
            std::cout << "Collision [" << std::hex << kv.first << "]: " << std::endl;
            for (auto& s : kv.second)
                std::cout << "\t" << s << std::endl;
        }
    }
*/