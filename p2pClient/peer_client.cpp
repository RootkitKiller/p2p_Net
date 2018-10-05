//
// Created by mooninwater on 2018/10/2.
//

#include <deque>
#include <vector>
#include <iostream>
#include "peer_client.h"


class connection :public std::enable_shared_from_this<connection>{
private:
    struct message_enqueue{                                         //消息队列结构
        unsigned char* p_buffer;
        uint32_t size;
        std::function<void(boost::system::error_code,std::size_t)> callback;
    };
public:
    connection(std::string addr,uint32_t port,peer_client& cli):peer_addr(addr),peer_port(port){
        p_socket=std::make_shared<ip::tcp::socket>(cli.get_io_service());
        p_endpoint=std::make_shared<ip::tcp::endpoint>(ip::address_v4::from_string(addr),port);
    }
    bool connect_state= false;
    void connect_peer();
    void add_msg_to_enqueues(message_enqueue& msg_handler);
    void send_msg_from_enqueues();

    void handler_hello_message(boost::system::error_code ec);
    void handler_read_message(boost::system::error_code ec, std::size_t size,\
                    unsigned char*  p_read_buffer,\
                    std::shared_ptr<ip::tcp::socket> p_data_socket);

    void read_msg_from_socket(std::shared_ptr<ip::tcp::socket> p_data_socket);
private:
    std::string peer_addr;
    uint32_t peer_port;
    std::shared_ptr<ip::tcp::endpoint> p_endpoint;

    std::shared_ptr<ip::tcp::socket> p_socket;          //通信的套接字

    std::deque<message_enqueue> msg_enqueues;           //消息队列

};

void connection::handler_read_message(boost::system::error_code ec, std::size_t size,\
                            unsigned char* p_read_buffer,\
                            std::shared_ptr<ip::tcp::socket> p_data_socket) {

    if(ec && ec!=boost::asio::error::eof){
        printf("wrong:%s\n",ec.message().c_str());
    } else{
        for(std::size_t i=0;i<size;i++){
            printf("%02x ",p_read_buffer[i]);
            if(i%0x10==0&&i!=0){
                printf("\n");
            }
        }
    }
    delete[] p_read_buffer;
}

void connection::handler_hello_message(boost::system::error_code ec) {
    if(ec){
        printf("wrong:%s\n",ec.message().c_str());
    }else{

        connect_state=true;
        std::string ss="hello world";
        auto buf_size=ss.size();

        unsigned char* p_buf_msg=new unsigned char[buf_size];
        for(auto i=0;i<ss.size();i++){
            p_buf_msg[i]=(ss[i]);
        }
        std::shared_ptr<message_enqueue> p_msg_handler=std::make_shared<message_enqueue>();
        p_msg_handler->p_buffer=p_buf_msg;
        p_msg_handler->size=buf_size;
        p_msg_handler->callback=[this,p_buf_msg](boost::system::error_code ec,std::size_t sz){
            printf("send to ip:%s port:%d message success !!!\n",this->p_endpoint->address().to_string().c_str(),\
                                                    this->p_endpoint->port());
            delete[] p_buf_msg;
        };
        add_msg_to_enqueues(*p_msg_handler);
    }

}

void connection::connect_peer() {
    p_socket->async_connect(*p_endpoint,std::bind(&connection::handler_hello_message,shared_from_this(),std::placeholders::_1));

}

void connection::add_msg_to_enqueues(message_enqueue& msg_handler) {
    msg_enqueues.push_front(msg_handler);
    send_msg_from_enqueues();
}

void connection::send_msg_from_enqueues() {
    //将buffer的内容转换为boost::asio::buffer
    std::vector<boost::asio::const_buffer> buffer;
    std::shared_ptr<std::vector<std::function<void(boost::system::error_code,std::size_t)>>> callbacks=\
                    std::make_shared<std::vector<std::function<void(boost::system::error_code,std::size_t)>>>();
    while(msg_enqueues.size()>0){
        auto &msg_enq= msg_enqueues.front();
        buffer.push_back(boost::asio::buffer((void*)(msg_enq.p_buffer),msg_enq.size));
        callbacks->push_back(msg_enq.callback);
        msg_enqueues.pop_front();
    }
    async_write(*p_socket,buffer,[this,callbacks](boost::system::error_code ec, std::size_t sz){
        if(ec&&ec!=boost::asio::error::eof){
            printf("send wrong!!\n");
        }else{
            for(auto &callback:*callbacks){
                callback(ec,sz);
            }
        }
    });
}


void connection::read_msg_from_socket(std::shared_ptr<ip::tcp::socket> p_data_socket) {
    unsigned char* p_read_buffer=new unsigned char[0x1000];
    async_read(*p_data_socket,boost::asio::buffer(p_read_buffer,0x1000),std::bind(&connection::handler_read_message,\
                                        shared_from_this(),std::placeholders::_1,\
                                        std::placeholders::_2,p_read_buffer,p_data_socket));
}

void peer_client::listen_loop() {
    std::shared_ptr<ip::tcp::socket> p_data_socket=std::make_shared<ip::tcp::socket>(*p_io_service);
    p_acceptor->async_accept(*p_data_socket,[p_data_socket,this](boost::system::error_code ec){
        if(ec){
            printf("wrong:%s\n",ec.message().c_str());
        }else{
            this->listen_loop();
            std::shared_ptr<connection> c=std::make_shared<connection>(\
                            p_data_socket->remote_endpoint().address().to_string(),\
                            p_data_socket->remote_endpoint().port(),*this);
            c->read_msg_from_socket(p_data_socket);
        }
    });
}

void peer_client::connect_peer() {
    for (const auto &peer:this->peer_list) {
        if (self_addr == peer.host_addr &&
            self_port == peer.host_port) {
            printf("self  addr:%s  port:%u\n", peer.host_addr.c_str(), peer.host_port);
        } else {
            printf("other addr:%s  port:%u\n", peer.host_addr.c_str(), peer.host_port);
            std::shared_ptr<connection> c=std::make_shared<connection>(peer.host_addr,peer.host_port,*this);
            //连接其他节点 并发送消息
            c->connect_peer();
        }
    }

}

void peer_client::get_peer_list() {
    //1 创建套接字
    std::shared_ptr<ip::tcp::socket> p_socket=std::make_shared<ip::tcp::socket>(*p_io_service);
    //2 连接到seed节点 获取节点列表
    ip::tcp::endpoint server_endp(ip::address_v4::from_string("127.0.0.1"),9400);
    p_socket->async_connect(server_endp,[p_socket,this](boost::system::error_code ec) {
        if (ec) {
            printf("wrong:%s\n", ec.message().c_str());
        } else {
            //1 异步从seed服务器获取peer list
            //std::string message="hello seed server!";
            char *message = new char[0x1000];
            boost::asio::async_read(*p_socket, boost::asio::buffer((void *) message, 0x1000), [p_socket, message, this]\
(boost::system::error_code ec, std::size_t size) {
                if (ec && ec != boost::asio::error::eof) {
                    printf("wrong,reason:%s\n,read number:%lu", ec.message().c_str(), size);
                } else {
                    std::size_t peer_size = size - sizeof(std::size_t);

                    for (std::size_t i = sizeof(std::size_t); i < size;) {
                        uint32_t addr_binary = *((uint32_t *) (message + i));
                        char addr_str[15];
                        if (inet_ntop(AF_INET, (void *) &addr_binary, addr_str, 15) != nullptr) {
                            std::string addr = addr_str;
                            i = i + 4;
                            uint32_t port = *(uint32_t *) (message + i);
                            i = i + 4;
                            this->peer_list.push_back({addr, port});
                        } else {
                            return;
                        }
                    }
                    this->self_addr=p_socket->local_endpoint().address().to_string();
                    this->self_port=p_socket->local_endpoint().port();
                    p_socket->close();
                    //1、向其他对等节点发送的内容
                    //ip::udp::endpoint udp_endpoint()
                    ip::tcp::endpoint endpoint(ip::address_v4::from_string("0.0.0.0"),self_port);
                    this->p_acceptor->open(endpoint.protocol());
                    this->p_acceptor->set_option(ip::tcp::acceptor::reuse_address(true));
                    this->p_acceptor->bind(endpoint);
                    this->p_acceptor->listen();
                    this->listen_loop();
                    //2、连接到其他节点
                    this->connect_peer();
                }
            });
        }
    });
}

void peer_client::start_up() {
    get_peer_list();
    p_io_service->run();
}
