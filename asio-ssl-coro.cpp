#include <iostream>
#include <thread>
#include <vector>

#define BOOST_ALLOW_DEPRECATED_HEADERS 1

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/signal_set.hpp>

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>

#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "uri.hpp"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

class session
{
public:
    template<typename _Arg>
    session(_Arg& ctx)
    :   _sslctx(asio::ssl::context::tlsv13_client),
        _socket(ctx, _sslctx)
    {
        _sslctx.set_verify_mode(asio::ssl::verify_none);
        _sslctx.set_options(boost::asio::ssl::context::default_workarounds |
                        boost::asio::ssl::context::no_sslv2 |
                        boost::asio::ssl::context::no_sslv3 |
                        boost::asio::ssl::context::single_dh_use |
                        boost::asio::ssl::context::no_tlsv1 |
                        boost::asio::ssl::context::no_tlsv1_1);

        SSL_CTX_set_alpn_protos(_sslctx.native_handle(),
            reinterpret_cast<const unsigned char*>("\x8http/1.1"), 9);

        SSL_CTX_set_session_cache_mode(_sslctx.native_handle(),
            SSL_SESS_CACHE_CLIENT | SSL_SESS_CACHE_NO_INTERNAL_STORE);

        SSL_CTX_set_options(_sslctx.native_handle(), SSL_OP_ALL);
    }

    void start(asio::ip::tcp::endpoint e, const std::string& host, const std::string& uri, asio::yield_context yield)
    {
        _socket.lowest_layer().async_connect(e, yield);
        std::cout << std::endl << "connected to " << e << std::endl;

        SSL_set_tlsext_host_name(_socket.native_handle(), host.c_str());
        _socket.async_handshake(asio::ssl::stream_base::client, yield);

        http::request<http::string_body> request(http::verb::get, uri, 11);
        request.set(http::field::host, host);
        request.set(http::field::user_agent, "Mozilla/5.0");
        request.set(http::field::accept, "*/*");

        http::async_write(_socket, request, yield);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> response;
        http::async_read(_socket, buffer, response, yield);

        std::cout << "code: " << response.result_int() << std::endl;

        beast::error_code ec;
        _socket.shutdown(ec);
    }

private:
    using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;

    asio::ssl::context  _sslctx;
    ssl_stream          _socket;
    std::string         _host;
};

template<typename _Arg, typename _Handler>
void resolve(_Arg& ctx, const std::string& host, const std::string& service,
        asio::yield_context yield, _Handler handler)
{
    try
    {
        asio::ip::tcp::resolver resolver(ctx);
        auto rr = resolver.async_resolve(host, service, yield);
        std::cout << host << " resolved to:" << std::endl;
        for(auto r : rr)
            std::cout << r.endpoint() << std::endl;

        handler(rr);
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}

template<typename _Arg>
void request(_Arg& ctx, const asio::ip::tcp::resolver::results_type& rr, int num,
        const std::string& host, const std::string& uri, asio::yield_context yield)
{
    std::mutex mtx;
    std::condition_variable cv;
    int active = num;
    for(int i = 0; i < num; ++i)
    {
        asio::spawn(ctx,
            [&](asio::yield_context yield)
            {
                for(auto r : rr)
                {
                    try
                    {
                        session s(ctx);
                        s.start(r.endpoint(), host, uri, yield);
                        break;
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << "Request to " << r.endpoint() << " failed: " << e.what() << std::endl;
                    }
                }

                std::unique_lock<std::mutex> lock(mtx);
                if(--active == 0)
                    cv.notify_one();
            });
    }

    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&active] { return active == 0; });
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 1 || argc > 3)
        {
            std::cerr << "Usage: https-coro <host>[:<port>][/<path>] [num]" << std::endl;
            std::cerr << "  <port> - https by default" << std::endl;
            std::cerr << "  <uri>  - / by default" << std::endl;
            return 1;
        }

        core::uri u = argv[1];
        if(u.service().empty())
            u.service("443");
        if(u.path().empty())
            u.path("/");

        int num = 1;
        if(argc == 3)
            num = std::stoi(argv[2]);

        asio::system_context context;

        asio::signal_set signals(context, SIGINT, SIGTERM);
        signals.async_wait([&](const std::error_code& ec, int signum)
        {
            if(ec)
                return;
            context.stop();
        });

        asio::spawn(context,
            [&](asio::yield_context yield)
            {
                resolve(context, u.host(), u.service(), yield,
                    [&](const asio::ip::tcp::resolver::results_type& rr)
                    {
                        request(context, rr, num, u.host(), u.path(), yield);
                    });
                context.stop();
            });

        context.join();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
