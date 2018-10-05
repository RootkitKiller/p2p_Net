
project(p2pclient)

set(CMAKE_CXX_STANDARD 11)

add_executable(p2pclient main.cpp peer_client.cpp peer_client.h)

target_link_libraries(p2pclient boost_system)