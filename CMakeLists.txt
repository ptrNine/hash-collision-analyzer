cmake_minimum_required(VERSION 3.13)
project(hash_collision)

set(CMAKE_CXX_STANDARD 17)

add_executable(hash_collision main.cpp)

target_link_libraries(hash_collision -pthread)