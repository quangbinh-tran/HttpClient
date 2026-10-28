#ifndef CURLTRANSPORT_HPP
#define CURLTRANSPORT_HPP

#include "ITransport.hpp"
#include <memory>
#include <mutex>

namespace http_client
{
class CurlTransport : public ITransport
{
public:
    CurlTransport();
    ~CurlTransport() override;
    HttpResponse request(const HttpRequest& request) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::mutex requestMutex_;
};
} // namespace http_client

#endif // CURLTRANSPORT_HPP