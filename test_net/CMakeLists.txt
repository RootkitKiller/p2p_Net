cmake_minimum_required(VERSION 3.5)

project(test_net)

set(CMAKE_CXX_STANDARD 11)

add_executable(test_net main.cpp)

target_link_libraries(test_net boost_system)