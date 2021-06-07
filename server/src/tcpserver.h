#ifndef TCPSERVER_H__
#define TCPSERVER_H__

#include <asio.hpp>

class TcpServer
{
public:
    TcpServer(asio::io_context&, asio::ip::tcp::endpoint);

private:
    void Accept();

private:
    asio::ip::tcp::acceptor acceptor_;
};

#endif /* TCPSERVER_H__ */
