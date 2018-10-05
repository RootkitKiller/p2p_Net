//
// Created by mooninwater on 2018/10/2.
//

#ifndef P2PNET_PEER_CLIENT_H
#define P2PNET_PEER_CLIENT_H

#include "boost/asio.hpp"

using namespace boost::asio;

class connection;

class peer_client {
private:
    struct peer{
        std::string host_addr;
        uint32_t host_port;
        peer(std::string addr,uint32_t port):host_addr(addr),host_port(port){}
    };
    std::vector<peer> peer_list;
    std::string self_addr;
    uint32_t self_port;
    std::shared_ptr<ip::tcp::socket> p_socket;
    std::shared_ptr<ip::tcp::acceptor> p_acceptor;
    void get_peer_list();
    void connect_peer();
    void listen_loop();
public:
    peer_client(){
        p_io_service=std::make_shared<io_service>();
        p_socket=std::make_shared<ip::tcp::socket>(*p_io_service);
        p_acceptor=std::make_shared<ip::tcp::acceptor>(*p_io_service);
    }
    std::shared_ptr<io_service> p_io_service;
    void start_up();
    io_service& get_io_service(){
        return *p_io_service;
    }
};

#endif //P2PNET_PEER_CLIENT_H
