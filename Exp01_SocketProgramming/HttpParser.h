#pragma once

#include <string>
#include <sstream>
#include <map>

class HttpParser
{
    std::map<std::string, std::string>  int_buf;    // internal storage
public:
    HttpParser();
    static std::string GetURL(const std::string &str_str);
    static std::string GetContentTypeFromURL(const std::string &url);
    HttpParser & Parse(const std::string &src_str);    // NOTE: copy-constructed
    virtual ~HttpParser();
};
