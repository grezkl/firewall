
#include <asio.hpp>
#include <iostream>

#include "tcpconnection.h"
#include "tcpserver.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cout << "usage: srv <host> <port>\n";
        return 1;
    }

    try
    {
        asio::io_context io;
        asio::ip::tcp::endpoint ep(asio::ip::make_address(argv[1]),
                                   stoi(argv[2]));
        TcpServer srv(io, ep);
        io.run();
    }
    catch (exception& e)
    {
        cerr << e.what() << "\n";
    }
    return 0;
}
