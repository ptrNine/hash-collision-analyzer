#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace hcutils {

    inline auto read_file_to_buffer(const std::string& path) -> std::vector<char> {
        auto ifs = std::ifstream(path, std::ios::in);

        if (!ifs.is_open()) {
            std::cerr << "Can't open file " << path << std::endl;
            return {};
        }

        std::vector<char> buffer;
        ifs.seekg(0, std::ios::end);
        auto last = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        size_t filesize = static_cast<size_t>(last - ifs.tellg());

        buffer.resize(filesize);
        ifs.read(buffer.data(), filesize);

        return std::move(buffer);
    }

} // namespace utils