#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <stdexcept>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

class ssl_session
{
public:
    ssl_session(asio::io_context& ctx)
    :   _c(ctx)
    {}

private:
    asio::io_context& _c;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: ssl-coro <host>\n";
            return 1;
        }

        asio::io_context io_context;
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
