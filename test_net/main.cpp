#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio::ip;

int main(int argc,char* argv[]){
    std::shared_ptr<boost::asio::io_service> io;
    io = std::make_shared<boost::asio::io_service>();

    std::shared_ptr<tcp::socket> s;
    s =  std::make_shared<tcp::socket>(*io);

    std::string arg=argv[1];
    uint32_t port=atoi(arg.c_str());
    printf("%d\n",port);

    tcp::endpoint server_endp(address_v4::from_string("127.0.0.1"),port);

    //s.connect(server_endp);
    s->async_connect(server_endp,[](boost::system::error_code ec){
        if(ec){
            printf("wrong:%s\n",ec.message().c_str());
        }else{
            printf("right\n");
        }
    });
    //(&s)->async_connect(server_endp,[](boost::system::error_code ec){});
    //(&s)->async_connect(server_endp,[](boost::system::error_code ec){});
    io->run();
    io->run();
    io->run();
    io->run();
}