#pragma once

#include <thread>
#include <functional>
#include <map>
#include <chrono>

#include "type_traits.hpp"
#include "thread_safe_map.hpp"
#include "utils.hpp"

#define HF(func) func, #func
#define HFN(func, name) func, name

namespace hctest {

    template <typename T>
    inline auto _get_func_or_str(T val)
    -> std::enable_if_t<!std::is_function_v<std::remove_pointer_t<T>>, std::string> {
        return val;
    }

    template <typename T>
    inline auto _get_func_or_str(T val)
    -> std::enable_if_t<std::is_function_v<std::remove_pointer_t<T>>, decltype(std::function(val))>  {
        return std::function(val);
    }

    template <typename... ArgT>
    inline auto generate_hct_tuple(ArgT... argv) {
        return std::make_tuple(_get_func_or_str(argv)...);
    }


    template <typename TupleFuncT>
    class HashCollisionTest {
    public:
        static_assert(traits::is_tuple_v<TupleFuncT>, "Not a valid HashCollisionTest tuple");

    public:
        using StringArrayT   = std::array<std::string, std::tuple_size_v<TupleFuncT>>;
        using StringVecT     = std::vector<std::string>;
        using SwitchRunFuncT = std::function<int(const StringVecT&, HashCollisionTest& hct)>;

    public:
        explicit HashCollisionTest(TupleFuncT tuple_f): _tuple_funcs (std::move(tuple_f)) {
            _check_tuple_f();
        }

        int run(int argc = 0, char* argv[] = nullptr) {
            StringVecT args;
            args.reserve(static_cast<std::size_t>(argc > 0 ? argc - 1 : argc));

            for (int i = 1; i < argc; ++i)
                args.emplace_back(argv[i]);

            if (!_process_args(args))
                return -1;

            if (args.empty())
                args.emplace_back("--help");

            return _switched_run(args);
        }

    protected:
        bool _process_args(StringVecT &args) {
            std::vector<std::size_t> to_delete;

            for (std::size_t i = 0; i < args.size(); ++i) {
                auto& arg = args[i];

                if (arg[0] == '-') {
                    for (auto c = arg.begin() + 1; c != arg.end(); ++c) {
                        // Todo: Worst. Refactor this!
                        // Jobs
                        if (*c == 'j') {
                            if (arg.end() - c  < 2) {
                                if (args.size() != i + 1) {
                                    try {
                                        _nprocs = std::stoul(args[i + 1]);
                                        to_delete.push_back(i + 1);
                                        ++i;
                                    }
                                    catch (std::invalid_argument &e) {
                                        _nprocs = std::thread::hardware_concurrency();
                                    }
                                } else
                                    _nprocs = std::thread::hardware_concurrency();
                            } else {
                                try {
                                    _nprocs = std::stoul(arg.substr(static_cast<std::size_t>((c + 1) - arg.begin())));
                                    break;
                                }
                                catch (std::invalid_argument &e) {
                                    _nprocs = std::thread::hardware_concurrency();
                                }
                            }
                            if (_nprocs == 0)
                                _nprocs = 1;
                        } // Jobs
                        else if (*c == 's') {
                            _silent = true;
                        }
                        else if (arg != "--help") {
                            std::cerr << "Unknown option " << arg << std::endl;
                            _show_help();
                            return false;
                        }
                    }

                    to_delete.push_back(i);
                }
            }

            for (std::size_t i = 0; i < to_delete.size(); ++i)
                args.erase(args.begin() + to_delete[i] - i);

            return true;
        }

        auto _divide_string_buf(const char *beg, const char *end, std::size_t num) -> std::vector<const char*> {
            std::vector<const char*> positions;

            auto ptr = beg;
            std::size_t part_dist = (end - ptr) / num;

            positions.push_back(ptr);
            for (std::size_t i = 1; i < num; ++i) {
                ptr += part_dist;

                while (*ptr != '\n' && ptr < end)
                    ++ptr;
                ++ptr;

                if (ptr < end)
                    positions.push_back(ptr);
                else
                    break;
            }

            return positions;
        };

        template <typename HashFuncT, typename HashMapT>
        static void _run_calc(HashFuncT& hash_func, HashMapT& hm, const char* start, const char* end) {
            std::string str;
            str.reserve(64);

            while (start < end) {
                str.clear();

                while(*start != '\n')
                    str.push_back(*start++);

                start++;

                auto hash = hash_func(str.c_str());

                if constexpr (std::is_integral_v<decltype(hash)>) {
                    if (!hm.find_fn(hash, [&](StringVecT &strvec) {
                        strvec.push_back(str);
                    }))
                        hm.insert(hash, StringVecT{str});
                } else {
                    hm.push(hash, str);
                }
            }
        }

        template <typename HashFuncT, typename HashMapT>
        void _run_multithread_calc(HashFuncT& hash_func, HashMapT& hm, std::vector<const char*> divided_strs, const char* end) {
            std::vector<std::thread> threads;
            threads.reserve(divided_strs.size());
            std::cerr << "Launch at " << divided_strs.size() << " threads..." << std::endl;

            auto run_calc_f = [](HashFuncT& hash_func, HashMapT& hm, const char* start, const char* end) {
                _run_calc(hash_func, hm, start, end);
            };

            for (std::size_t i = 0; i < divided_strs.size(); ++i) {
                if (i != divided_strs.size() - 1)
                    threads.emplace_back(std::thread(run_calc_f, std::ref(hash_func), std::ref(hm), divided_strs[i], divided_strs[i + 1]));
                else
                    threads.emplace_back(std::thread(run_calc_f, std::ref(hash_func), std::ref(hm), divided_strs[i], end));
            }

            for (auto& t : threads)
                t.join();
        }

        template <typename HashValT>
        static auto _get_map_impl() {
            if constexpr (std::is_integral_v<HashValT>)
                return cuckoohash_map<HashValT, StringVecT>();
            else
                return hcutils::TSVectorMap<HashValT, std::string>();
        }

        template <std::size_t _HashFuncNum>
        int _process_main(const StringVecT &args) {
            if (args.size() < 2) {
                std::cerr << "Missing file path" << std::endl;
                _show_help();
                return -1;
            }

            auto buf = hcutils::read_file_to_buffer(args[1]);

            if (buf.empty()) {
                return -1;
            }

            const char* start = buf.data();
            const char* end   = buf.data() + buf.size();


            auto& hash_func = std::get<_HashFuncNum + 0>(_tuple_funcs);
            auto& hash_name = std::get<_HashFuncNum + 1>(_tuple_funcs);

            using HashT = traits::return_type_of<decltype(hash_func)>;


            auto map_impl = _get_map_impl<HashT>();

            auto time = std::chrono::high_resolution_clock::now();

            if (_nprocs == 1)
                _run_calc(hash_func, map_impl, start, end);
            else {
                auto divide_strs = _divide_string_buf(start, end, _nprocs);
                _run_multithread_calc(hash_func, map_impl, divide_strs, end);
            }

            auto elapsed = std::chrono::high_resolution_clock::now() - time;

            std::cerr << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0 << "s" << std::endl;

            std::cout << "Results for " << hash_name << std::endl;

            if constexpr (std::is_integral_v<HashT>) {
                std::size_t collisions_count = 0;
                auto locked_table = map_impl.lock_table();

                for (auto &h : locked_table)
                    if (h.second.size() > 1)
                        collisions_count++;

                std::cout << "Total collisions count: " << collisions_count << std::endl;

                if (!_silent) {
                    for (auto &h : locked_table) {
                        if (h.second.size() > 1) {
                            std::cout << "Collision [" << std::hex << h.first << "]:" << std::endl;

                            for (auto &s : h.second)
                                std::cout << "\t" << s << std::endl;
                        }
                    }
                }
            }
            else {
                std::size_t collisions_count = 0;
                for (auto &h : map_impl)
                    if (h.second.size() > 1)
                        collisions_count++;

                std::cout << "Total collisions count: " << collisions_count << std::endl;

                if (!_silent) {
                    for (auto &h : map_impl) {
                        if (h.second.size() > 1) {
                            std::cout << "Collision [" << std::hex << h.first << "]:" << std::endl;

                            for (auto &s : h.second)
                                std::cout << "\t" << s << std::endl;
                        }
                    }
                }
            }

            return 0;
        }

        void _show_help() {
            std::cerr << "\nUsage: hash_collision [options]... [hash function name] [path/to/file]" << std::endl;
            std::cerr << "\nOptions: \n"
                         "\t-j [N]     Run at N jobs. If no arg provided - run with std::thread::hardware_concurrency()\n"
                         "\t-s         No information about collisions\n";
            std::cerr << std::endl;
        }

        void _show_help(const StringVecT& hash_func_names) {
            _show_help();
            std::cerr << "Available hash functions:\n";
            for (auto& hsn : hash_func_names)
                std::cerr << "\t" << hsn << std::endl;
        }

        template <std::size_t _Iter = 0>
        auto _switched_run_cases(const StringVecT& args, StringVecT& available_hash_funcs)
        -> std::enable_if_t<(_Iter < std::tuple_size_v<TupleFuncT>), int>
        {
            if (available_hash_funcs.emplace_back(std::get<_Iter + 1>(_tuple_funcs)),
                args[0] == std::get<_Iter + 1>(_tuple_funcs))
            {
                return _process_main<_Iter>(args);
            }
            else
                return _switched_run_cases<_Iter + 2>(args, available_hash_funcs);
        }

        template <std::size_t _Iter>
        auto _switched_run_cases(const StringVecT& args, StringVecT& available_hash_funcs)
        -> std::enable_if_t<(_Iter >= std::tuple_size_v<TupleFuncT>), int> {
            if (args[0] == "--help") {
                _show_help(available_hash_funcs);
                return 0;
            }
            else {
                std::cerr << "Unknown hash function name " << args[0] << std::endl;
                _show_help(available_hash_funcs);
                return -1;
            }
        }

        int _switched_run(const StringVecT& args) {
            StringVecT available_hash_funcs;
            return _switched_run_cases(args, available_hash_funcs);
        }

        template <std::size_t _Iter = 0, std::size_t _Size = std::tuple_size_v<TupleFuncT>>
        static constexpr auto _check_tuple_f() -> std::enable_if_t<_Iter != _Size>
        {
            using FunctT =
                    std::remove_reference_t<
                            decltype(std::get<_Iter>(std::declval<TupleFuncT>()))>;

            static_assert(traits::args_count<FunctT> == 1,
                          "Wrong arguments count. Hash function must accept one const char* string.");
            static_assert(std::is_same_v<traits::arg_type_of<FunctT, 0>, const char*>,
                          "Invalid argument type. Hash function must accept one const char* string.");

            _check_tuple_f<_Iter + 2, _Size>();
        }

        template <std::size_t _Iter = 0, std::size_t _Size>
        static constexpr auto _check_tuple_f() -> std::enable_if_t<_Iter == _Size> {}

    private:
        TupleFuncT _tuple_funcs;

        std::size_t _nprocs = 1;
        bool        _silent = false;
    };

} // namespace hctest