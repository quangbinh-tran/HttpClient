#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP
#include <map>
#include <string>
#include <chrono>

enum class HttpMethod
{
    GET,
    POST,
    PUT,
    DELETE,
    PATCH
};

namespace http_client
{
    struct HttpRequest
    {
        HttpMethod method{HttpMethod::GET};

        std::string url;

        std::map<std::string, std::string> headers;

        std::string body;

        std::chrono::milliseconds timeout{0}; // Timeout in milliseconds, 0 means no timeout
    };
} // namespace http_client

#endif // HTTP_REQUEST_HPP