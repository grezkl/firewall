#include "tcpconnection.h"
#include <fstream>
#include <iostream>

using asio::ip::tcp;

TcpConnection::TcpConnection(tcp::socket socket)
    : socket_(std::move(socket)) { }

void TcpConnection::Start()
{
    sc_.Connect();
    Read();
}

void TcpConnection::Process()
{
    // 此处可以添加对客户端发送数据的处理，目前没需求
    // ...

    // 执行数据库操作并返回结果
    sc_.Process();
}

void TcpConnection::Read()
{
    auto self(shared_from_this());

    socket_.async_read_some(
        asio::buffer(recv_buf_, max_length),
        [self, this](const std::error_code& ec, std::size_t len)
        {
            if (ec)
            {
                std::cerr << ec.message() << "\n";
                return;
            }

            // 循环进行读取、处理、写入（给客户端）操作
            std::cout << "[recv]" << recv_buf_.data();
            Process();
            bzero(recv_buf_.data(), max_length);
            Write(len);
        });
}

void TcpConnection::Write(size_t len)
{
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(sc_.GetResult()),
                      [self, this](const std::error_code& ec, std::size_t len)
                      {
                          if (ec)
                          {
                              std::cerr << "write err: " << ec.message()
                                        << "\n";
                              return;
                          }
                          std::cout << "[send]" << sc_.GetResult() + "\0";
                          socket_.shutdown(tcp::socket::shutdown_both);
                          Read();
                      });
}
