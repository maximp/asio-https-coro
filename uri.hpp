#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace core {

    class uri
    {
    public:
        uri();
        uri(const char* u);
        uri(std::string_view u);

        uri(const uri&) = default;
        uri(uri&&) = default;

        uri& operator=(const uri&) = default;
        uri& operator=(uri&&) = default;

        const std::string& scheme() const { return _scheme; }
        const std::string& host() const { return _host; }
        const std::string& service() const { return _service; }
        const std::string& path() const { return _path; }

        void scheme(const std::string& v) { _scheme = v; }
        void host(const std::string& v) { _host = v; }
        void service(const std::string& v) { _service = v; }
        void path(const std::string& v) { _path = v; }

        bool is_https() const { return scheme() == "https"; }

        unsigned short port(unsigned short def = 0) const;
        bool valid() const { return !_host.empty(); }

        std::string string() const;

    private:
        std::string _scheme;
        std::string _host;
        std::string _service;
        std::string _path;

        static std::string_view trim_last_slash(std::string_view s);
    };

    inline bool operator==(const uri& left, const uri& right)
    {
        return left.scheme() == right.scheme() &&
               left.host() == right.host() &&
               left.service() == right.service() &&
               left.path() == right.path();
    }

    inline bool operator!=(const uri& left, const uri& right) {
        return !operator==(left, right);
    }

    inline std::ostream& operator<<(std::ostream& os, const uri& u) {
        return os << u.string();
    }
}
