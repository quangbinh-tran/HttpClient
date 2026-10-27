#include <gtest/gtest.h>

#include "HttpClient.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>
#include <string>
#include <thread>

namespace
{
class LocalHttpServer
{
public:
    explicit LocalHttpServer(std::string response)
        : response_(std::move(response))
    {
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ < 0)
        {
            throw std::runtime_error("Failed to create test server socket");
        }

        int reuseAddress = 1;
        setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                   sizeof(reuseAddress));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(0);

        if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&address),
                 sizeof(address)) < 0 ||
            listen(serverSocket_, 1) < 0)
        {
            close(serverSocket_);
            throw std::runtime_error("Failed to start test server");
        }

        socklen_t addressLength = sizeof(address);
        if (getsockname(serverSocket_, reinterpret_cast<sockaddr*>(&address),
                        &addressLength) < 0)
        {
            close(serverSocket_);
            throw std::runtime_error("Failed to get test server port");
        }

        port_ = ntohs(address.sin_port);
        serverThread_ = std::thread([this] { serveOneRequest(); });
    }

    ~LocalHttpServer()
    {
        if (serverThread_.joinable())
        {
            serverThread_.join();
        }
        if (serverSocket_ >= 0)
        {
            close(serverSocket_);
        }
    }

    LocalHttpServer(const LocalHttpServer&) = delete;
    LocalHttpServer& operator=(const LocalHttpServer&) = delete;

    [[nodiscard]] unsigned short port() const noexcept
    {
        return port_;
    }

private:
    void serveOneRequest() noexcept
    {
        const int clientSocket = accept(serverSocket_, nullptr, nullptr);
        if (clientSocket < 0)
        {
            return;
        }

        char buffer[1024];
        recv(clientSocket, buffer, sizeof(buffer), 0);

        const std::string headers =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " +
            std::to_string(response_.size()) + "\r\n"
            "Connection: close\r\n\r\n";
        send(clientSocket, headers.data(), headers.size(), 0);
        send(clientSocket, response_.data(), response_.size(), 0);
        close(clientSocket);
    }

    std::string response_;
    int serverSocket_{-1};
    unsigned short port_{0};
    std::thread serverThread_;
};
} // namespace

TEST(HttpResponseTest, DefaultResponseIsNotSuccessful)
{
    const http_client::HttpResponse response;

    EXPECT_EQ(response.status_code, 0);
    EXPECT_FALSE(response.is_success());
    EXPECT_TRUE(response.body.empty());
    EXPECT_TRUE(response.error_message.empty());
}

TEST(HttpResponseTest, SuccessfulStatusCodesAreRecognized)
{
    http_client::HttpResponse response;

    response.status_code = 200;
    EXPECT_TRUE(response.is_success());

    response.status_code = 299;
    EXPECT_TRUE(response.is_success());

    response.status_code = 199;
    EXPECT_FALSE(response.is_success());

    response.status_code = 300;
    EXPECT_FALSE(response.is_success());
}

TEST(HttpClientTest, GetRequestReturnsStatusAndBody)
{
    LocalHttpServer server("hello from test server");
    http_client::HttpClient client;

    const auto response = client.request(
        HttpMethod::GET,
        "http://127.0.0.1:" + std::to_string(server.port()) + "/hello");

    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.body, "hello from test server");
    EXPECT_TRUE(response.error_message.empty());
    EXPECT_TRUE(response.is_success());
}

TEST(HttpClientTest, PostRequestReturnsStatusAndBody)
{
    LocalHttpServer server("post response");
    http_client::HttpClient client;

    const auto response = client.request(
        HttpMethod::POST,
        "http://127.0.0.1:" + std::to_string(server.port()) + "/submit",
        "request body", {{"Content-Type", "text/plain"}});

    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.body, "post response");
    EXPECT_TRUE(response.is_success());
}

TEST(HttpClientTest, InvalidUrlReturnsCurlError)
{
    http_client::HttpClient client;

    const auto response = client.request(HttpMethod::GET, "not a URL");

    EXPECT_EQ(response.status_code, 0);
    EXPECT_FALSE(response.error_message.empty());
    EXPECT_FALSE(response.is_success());
}
