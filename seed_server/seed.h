//
// Created by mooninwater on 2018/10/2.
//

#ifndef P2PNET_SEED_H
#define P2PNET_SEED_H


#include <string>
#include <list>
#include "boost/asio.hpp"
using namespace boost::asio;

class seed {
private:
    struct endpoint{
        std::string host_addr;
        uint32_t host_addr_binary;
        uint32_t host_port;
        endpoint(std::string addr,uint32_t addr_binary,uint32_t port):host_addr(addr),host_addr_binary(addr_binary),host_port(port){}
    };
    std::list<endpoint> endpoints;
    boost::asio::io_service io_service;             //异步对象
    void start_listen_loop();
    std::unique_ptr<ip::tcp::acceptor> p_accept;
public:
    void start();
};


#endif //P2PNET_SEED_H
