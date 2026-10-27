#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP
#include <map>
#include <string>

namespace http_client
{
    struct HttpResponse
    {
        long status_code{0};

        std::string body;

        std::map<std::string, std::string> headers;

        long timeout{0}; // Timeout in milliseconds, 0 means no timeout

        std::string error_message;

        [[nodiscard]]
        bool is_success() const noexcept
        {
            return status_code >= 200 && status_code < 300;
        }
    };
} // namespace http_client

#endif // HTTP_RESPONSE_HPP