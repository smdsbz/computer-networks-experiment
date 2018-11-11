#include "pch.h"
#include "HttpParser.h"

// Data Structures
using std::string; using std::map;
using std::istringstream; using std::getline;


/* Function Implementations */

HttpParser::HttpParser()
{
    return;
}


HttpParser::~HttpParser()
{
    return;
}


string HttpParser::GetURL(const string &src)
{
    string retstr;
    auto first_slash_idx = src.find('/');
    if (first_slash_idx == std::string::npos) {
        return string();    // empty string
    }
    auto second_space_idx = src.find(' ', first_slash_idx);
    if (second_space_idx == std::string::npos) {
        return string();
    }
    return src.substr(first_slash_idx, second_space_idx - first_slash_idx);
}


string HttpParser::GetContentTypeFromURL(const string &url)
{
    auto last_dot_idx = url.find_last_of('.');
    if (last_dot_idx == std::string::npos) {    // no dots found
        return string("text/html");
    }
    if (last_dot_idx == url.size() - 1) {   // url ends with dot
        return string();
    }
    string fileext = url.substr(last_dot_idx + 1);
    if ((fileext == string("jpeg"))
            || (fileext == string("jpg"))) {
        return string("image/jpeg");
    }
    if (fileext == string("pdf")) {
        return string("application/pdf");
    }
    if (fileext == string("html")) {
        return string("text/html");
    }
    return string("text/html");
}


HttpParser & HttpParser::Parse(const string &src)
{
    // TODO
    return *this;
}
