#ifndef TCPCONNECTION_H__
#define TCPCONNECTION_H__

#include "sqlconnection.h"
#include <asio.hpp>
#include <memory>
#include <string>

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(asio::ip::tcp::socket);
    void Start();

private:
    void Process();
    void Read();
    void Write(std::size_t);

private:
    asio::ip::tcp::socket socket_;
    SqlConnection sc_;
    enum { max_length = 1024 };
    std::array<char, max_length> recv_buf_;
};

#endif /* TCPCONNECTION_H__ */
