cmake_minimum_required(VERSION 3.24)
project(guanaqo_test_package CXX)

find_package(guanaqo REQUIRED)
add_executable(example src/example.cpp)
target_link_libraries(example PRIVATE guanaqo::guanaqo)
install(TARGETS example)
