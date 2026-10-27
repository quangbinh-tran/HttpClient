#ifndef ITRANSPORT_HPP
#define ITRANSPORT_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

namespace http_client
{
    class ITransport
    {
    public:
        virtual ~ITransport() = default;

        virtual HttpResponse request(const HttpRequest& request) = 0;
    };
} // namespace http_client

#endif // ITRANSPORT_HPP