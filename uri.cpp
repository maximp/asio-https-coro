#include "uri.hpp"

#include <cstring>
#include <sstream>

namespace core {

    uri::uri()
    {}

    uri::uri(const char* u)
    :   uri(std::string(u))
    {}

    uri::uri(std::string_view u)
    {
        bool found;
        std::string::size_type spos = 0;
        std::string::size_type epos = 0;

        auto cut_until = [&](const char* str)
        {
            std::string result;
            epos = u.find(str, spos);
            if(epos == std::string::npos)
            {
                epos = spos;
                found = false;
            }
            else
            {
                result = u.substr(spos, epos - spos);
                epos += std::strlen(str);
                spos = epos;
                found = true;
            }
            return result;
        };

        _scheme = cut_until("://");
        _host = cut_until(":");
        if(!found)
        {
            _host = cut_until("/");
            if(!found)
                _host = trim_last_slash(u.substr(spos));
            else
                _path = trim_last_slash(u.substr(spos - 1));
        }
        else
        {
            _service = cut_until("/");
            if(!found)
                _service = trim_last_slash(u.substr(spos));
            else
                _path = trim_last_slash(u.substr(spos - 1));
        }
    }

    std::string_view uri::trim_last_slash(std::string_view s)
    {
        if(s.empty())
            return s;

        for(std::size_t i = s.size(); --i >= 0; )
        {
            if(s[i] == '/')
                continue;

            if(i == s.size() - 1)
                return s;
            else
                return s.substr(0, i + 1);
        }
        return "";
    }

    unsigned short uri::port(unsigned short def) const
    {
        try
        {
            return static_cast<unsigned short>(std::stoul(_service));
        }
        catch(const std::exception&)
        {
        }

        return def;
    }

    std::string uri::string() const
    {
        std::ostringstream os;
        if(!_scheme.empty())
            os << _scheme << "://";
        if(!_host.empty())
            os << _host;
        if(!_service.empty())
            os << ":" << _service;
        if(!_path.empty())
            os << _path;
        return os.str();
    }
}
