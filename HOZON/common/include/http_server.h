#ifndef HOZON_HTTP_SERVER_H
#define HOZON_HTTP_SERVER_H

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include "json.h"

namespace hozon {

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    OPTIONS
};

struct HttpRequest {
    HttpMethod method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> pathParams;
    std::map<std::string, std::string> queryParams;

    nlohmann::json json() const;
};

struct HttpResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;

    HttpResponse();
    HttpResponse(int code, const std::string& body);

    static HttpResponse ok(const nlohmann::json& data);
    static HttpResponse created(const nlohmann::json& data);
    static HttpResponse badRequest(const std::string& message);
    static HttpResponse notFound(const std::string& message);
    static HttpResponse conflict(const std::string& message);
    static HttpResponse internalError(const std::string& message);
};

using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

class HttpServer {
public:
    HttpServer(int port);
    ~HttpServer();

    void get(const std::string& path, RouteHandler handler);
    void post(const std::string& path, RouteHandler handler);
    void put(const std::string& path, RouteHandler handler);
    void del(const std::string& path, RouteHandler handler);

    void start();
    void stop();
    bool isRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hozon

#endif // HOZON_HTTP_SERVER_H
