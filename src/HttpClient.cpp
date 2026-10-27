#include <HttpClient.hpp>
#include <CurlTransport.hpp>

namespace http_client
{
    class HttpClient::Impl
    {
    public:
        std::unique_ptr<ITransport> transport;
    };

    HttpClient::HttpClient() : impl_(std::make_unique<Impl>())
    {
        impl_->transport = std::make_unique<CurlTransport>();
    }

    HttpClient::~HttpClient() = default;

    HttpResponse HttpClient::request(
        HttpMethod method, 
        const std::string& url, 
        std::string_view body, 
        const std::map<std::string, std::string>& headers,
        std::chrono::milliseconds timeout)
    {
        HttpRequest request;
        request.method = method;
        request.url = url;
        request.body = std::string(body);
        request.headers = headers;
        request.timeout = timeout;

        return performRequest(request);
    }

    HttpResponse HttpClient::performRequest(const HttpRequest& request)
    {
        return impl_->transport->request(request);
    }
}
