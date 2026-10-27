#include "CurlTransport.hpp"
#include <curl/curl.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace http_client
{

namespace
{
struct CurlDeleter
{
    void operator()(CURL* curl) const noexcept
    {
        if (curl)
        {
            curl_easy_cleanup(curl);
        }
    }
};

struct CurlSlistDeleter
{
    void operator()(curl_slist* list) const noexcept
    {
        if (list)
        {
            curl_slist_free_all(list);
        }
    }
};

using CurlHandle = std::unique_ptr<CURL, CurlDeleter>;
using CurlSlistHandle = std::unique_ptr<curl_slist, CurlSlistDeleter>;

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userData)
{
    auto* buffer = static_cast<std::string*>(userData);
    buffer->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
}

class CurlTransport::Impl
{
public:
    CurlHandle curl;
};

CurlTransport::CurlTransport() : impl_(std::make_unique<Impl>())
{
    impl_->curl.reset(curl_easy_init());
    if (!impl_->curl)
    {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

CurlTransport::~CurlTransport() = default;

HttpResponse CurlTransport::request(const HttpRequest& request)
{
    HttpResponse response{};

    std::string responseBody;

    curl_easy_reset(impl_->curl.get());

    curl_easy_setopt(impl_->curl.get(), CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(impl_->curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(impl_->curl.get(), CURLOPT_WRITEDATA, &responseBody);

    if (request.timeout.count() > 0)
    {
        curl_easy_setopt(impl_->curl.get(), CURLOPT_TIMEOUT_MS, request.timeout.count());
    }

    CurlSlistHandle headerList;
    for (const auto& [name, value] : request.headers)
    {
        std::string header = name + ": " + value;
        curl_slist* appended = curl_slist_append(headerList.get(), header.c_str());
        if (!appended)
        {
            throw std::runtime_error("Failed to append CURL header");
        }
        headerList.reset(appended);
    }

    if (headerList)
    {
        curl_easy_setopt(impl_->curl.get(), CURLOPT_HTTPHEADER, headerList.get());
    }

    switch (request.method)
    {
        case HttpMethod::GET:
            curl_easy_setopt(impl_->curl.get(), CURLOPT_HTTPGET, 1L);
            break;
        
        case HttpMethod::POST:
            curl_easy_setopt(impl_->curl.get(), CURLOPT_POST, 1L);
            curl_easy_setopt(impl_->curl.get(), CURLOPT_POSTFIELDS, request.body.c_str());
            break;
        
        case HttpMethod::PUT:
            curl_easy_setopt(impl_->curl.get(), CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(impl_->curl.get(), CURLOPT_POSTFIELDS, request.body.c_str());
            curl_easy_setopt(impl_->curl.get(), CURLOPT_POSTFIELDSIZE, request.body.size());
            break;

        case HttpMethod::DELETE:
            curl_easy_setopt(impl_->curl.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        
        case HttpMethod::PATCH:
            curl_easy_setopt(impl_->curl.get(), CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(impl_->curl.get(), CURLOPT_POSTFIELDS, request.body.c_str());
            curl_easy_setopt(impl_->curl.get(), CURLOPT_POSTFIELDSIZE, request.body.size());
            break;
    }
    const CURLcode result = curl_easy_perform(impl_->curl.get());
    long statusCode = 0;
    curl_easy_getinfo(impl_->curl.get(), CURLINFO_RESPONSE_CODE, &statusCode);
    response.status_code = statusCode;
    response.body = std::move(responseBody);
    if (result != CURLE_OK)
    {
        response.error_message = curl_easy_strerror(result);
    }

    return response;
}

} // namespace http_client
