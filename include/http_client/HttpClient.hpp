#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace http_client
{
class HttpClient 
{
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpClient(HttpClient&&) = delete;
    HttpClient& operator=(HttpClient&&) = delete;

public:
    /**
     * @brief Sends an HTTP request.
     * @param method The HTTP method to use.
     * @param url The URL to send the request to.
     * @param body The body of the request.
     * @param headers The headers to include in the request.
     * @param timeout The timeout for the request, timeout in milliseconds, 0 means no timeout.
     * @return The response to the request.
     */
    HttpResponse request(
        HttpMethod method, 
        const std::string& url, 
        std::string_view body = {}, const std::map<std::string, std::string>& headers = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds{0});

private:
    HttpResponse performRequest(const HttpRequest& request);

    class Impl;
    std::unique_ptr<Impl> impl_;
};
}

#endif // HTTP_CLIENT_HPP