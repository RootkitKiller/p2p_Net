//
// Created by mooninwater on 2018/10/2.
//

#include "seed.h"


void seed::start_listen_loop() {
    //创建一个通信套接字
    std::shared_ptr<ip::tcp::socket> p_datasocket=std::make_shared<ip::tcp::socket>(io_service);
    p_accept->async_accept(*p_datasocket,[this,p_datasocket](boost::system::error_code ec){
        if(ec){
            printf("wrong:%s\n",ec.message().c_str());
        }else{
            printf("find new connect\n");
            auto addr_str=p_datasocket->remote_endpoint().address().to_string();
            uint32_t addrbuf;
            auto ret_value=inet_pton(AF_INET,addr_str.c_str(),(void*)&addrbuf);
            if(ret_value!=1){
                printf("wrong:inet_pton faild");
                return;
            }
            auto port_num=p_datasocket->remote_endpoint().port();
            printf("remote ip:%s\n",addr_str.c_str());
            printf("remote port:%d\n",port_num);
            endpoints.insert(endpoints.end(),{addr_str,addrbuf,port_num});

            //构造peer_list报文，将peer list 发送给客户端
            // message_size[8字节] | addr[4字节] | port[4字节] | addr[字节] | port[4字节] ........
            std::size_t size=sizeof(std::size_t)+endpoints.size()*8;
            using BYTE=unsigned char;
            BYTE* pbuf=new BYTE[size];
            BYTE* pbuf_backup=pbuf;
            *((std::size_t *)pbuf)=size;
            pbuf+=sizeof(std::size_t);

            for(const auto &iter:endpoints){
                *((uint32_t *)pbuf)=iter.host_addr_binary;
                pbuf+=sizeof(uint32_t);
                *((uint32_t *)pbuf)=iter.host_port;
                pbuf+=sizeof(uint32_t);
            }



            boost::asio::async_write(*p_datasocket,boost::asio::buffer((void*)pbuf_backup, size),\
                        [pbuf_backup,p_datasocket](const boost::system::error_code& error,std::size_t size){
                if(error) {
                    printf("wrong: write data err: %s\n", error.message().c_str());
                }
                delete[] pbuf_backup;
            });
        }
        //递归启动一个循环，等待下一个连接
        this->start_listen_loop();

    });
}

void seed::start() {
    //使用异步对象io_service 初始化一个接收器

    p_accept.reset(new ip::tcp::acceptor(io_service));

    //绑定到本地ip

    ip::tcp::endpoint endpoint(ip::address_v4::from_string("0.0.0.0"),9400);
    p_accept->open(endpoint.protocol());
    p_accept->set_option(ip::tcp::acceptor::reuse_address(true));  //设置地址复用
    p_accept->bind(endpoint);
    p_accept->listen();

    //接受客户端发来的连接
    start_listen_loop();
    io_service.run();
}
