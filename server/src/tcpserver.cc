#include "tcpserver.h"
#include "tcpconnection.h"

#include <iostream>

using asio::ip::tcp;

TcpServer::TcpServer(asio::io_context& io, tcp::endpoint ep) : acceptor_(io, ep)
{
    Accept();
}

void TcpServer::Accept()
{
    acceptor_.async_accept(
        [this](const std::error_code& ec, tcp::socket socket)
        {
            if (ec)
            {
                std::cerr << "accept error: " << ec.message() << "\n";
                return;
            }
            std::make_shared<TcpConnection>(std::move(socket))->Start();
            Accept();
        });
}
