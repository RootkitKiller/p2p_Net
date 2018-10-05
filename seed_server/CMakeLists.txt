
project(p2pseed)

set(CMAKE_CXX_STANDARD 11)

add_executable(p2pseed main.cpp seed.cpp seed.h)

target_link_libraries(p2pseed boost_system)