#include <map>
#include <vector>
#include <shared_mutex>

namespace hcutils {

    template<typename KeyT, typename ValT>
    class TSVectorMap {
    public:
        auto begin() const {
            auto lock = std::lock_guard(_mutex);
            return _map.begin();
        }

        auto end() const {
            auto lock = std::lock_guard(_mutex);
            return _map.end();
        }

        void push(const KeyT& key, const ValT& value) {
            auto lock = std::lock_guard(_mutex);
            _map[key].push_back(value);
        }

    private:
        std::map<KeyT, std::vector<ValT>> _map;

        mutable std::mutex _mutex;
    };

} // namespace hcutils