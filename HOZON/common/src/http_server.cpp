#include "http_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#include <algorithm>

namespace hozon {

nlohmann::json HttpRequest::json() const {
    if (body.empty()) {
        return nlohmann::json::object();
    }
    try {
        return nlohmann::json::parse(body);
    } catch (...) {
        return nlohmann::json::object();
    }
}

HttpResponse::HttpResponse() : statusCode(200), body("") {
    headers["Content-Type"] = "application/json";
}

HttpResponse::HttpResponse(int code, const std::string& b)
    : statusCode(code), body(b) {
    headers["Content-Type"] = "application/json";
}

HttpResponse HttpResponse::ok(const nlohmann::json& data) {
    return HttpResponse(200, data.dump());
}

HttpResponse HttpResponse::created(const nlohmann::json& data) {
    return HttpResponse(201, data.dump());
}

HttpResponse HttpResponse::badRequest(const std::string& message) {
    nlohmann::json j = {{"error", message}};
    return HttpResponse(400, j.dump());
}

HttpResponse HttpResponse::notFound(const std::string& message) {
    nlohmann::json j = {{"error", message}};
    return HttpResponse(404, j.dump());
}

HttpResponse HttpResponse::conflict(const std::string& message) {
    nlohmann::json j = {{"error", message}};
    return HttpResponse(409, j.dump());
}

HttpResponse HttpResponse::internalError(const std::string& message) {
    nlohmann::json j = {{"error", message}};
    return HttpResponse(500, j.dump());
}

struct Route {
    HttpMethod method;
    std::string pattern;
    std::regex regex;
    std::vector<std::string> paramNames;
    RouteHandler handler;
};

class HttpServer::Impl {
public:
    int port_;
    int serverSocket_;
    std::atomic<bool> running_;
    std::vector<Route> routes_;
    std::thread serverThread_;

    Impl(int port) : port_(port), serverSocket_(-1), running_(false) {}

    ~Impl() {
        stop();
    }

    void addRoute(HttpMethod method, const std::string& path, RouteHandler handler) {
        Route route;
        route.method = method;
        route.pattern = path;
        route.handler = handler;

        // Конвертация паттерна пути в regex: /orders/{id} -> /orders/([^/]+)
        std::string regexPattern = "^";
        std::string current = path;
        std::regex paramRegex("\\{([^}]+)\\}");
        std::smatch match;

        size_t lastPos = 0;
        std::string::const_iterator searchStart = current.cbegin();

        while (std::regex_search(searchStart, current.cend(), match, paramRegex)) {
            size_t matchPos = match.position(0) + (searchStart - current.cbegin());
            regexPattern += current.substr(lastPos, matchPos - lastPos);
            regexPattern += "([^/]+)";
            route.paramNames.push_back(match[1].str());
            lastPos = matchPos + match[0].length();
            searchStart = current.cbegin() + lastPos;
        }
        regexPattern += current.substr(lastPos);
        regexPattern += "$";

        route.regex = std::regex(regexPattern);
        routes_.push_back(route);
    }

    void start() {
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        int opt = 1;
        setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);

        if (bind(serverSocket_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            close(serverSocket_);
            throw std::runtime_error("Failed to bind to port " + std::to_string(port_));
        }

        if (listen(serverSocket_, 10) < 0) {
            close(serverSocket_);
            throw std::runtime_error("Failed to listen on socket");
        }

        running_ = true;
        serverThread_ = std::thread(&Impl::acceptLoop, this);

        std::cout << "HTTP Server started on port " << port_ << std::endl;
    }

    void stop() {
        if (running_) {
            running_ = false;
            if (serverSocket_ >= 0) {
                shutdown(serverSocket_, SHUT_RDWR);
                close(serverSocket_);
                serverSocket_ = -1;
            }
            if (serverThread_.joinable()) {
                serverThread_.join();
            }
        }
    }

    void acceptLoop() {
        while (running_) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);

            int clientSocket = accept(serverSocket_, (sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                if (running_) {
                    std::cerr << "Accept error" << std::endl;
                }
                continue;
            }

            std::thread([this, clientSocket]() {
                handleClient(clientSocket);
            }).detach();
        }
    }

    void handleClient(int clientSocket) {
        char buffer[8192] = {0};
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }

        std::string requestStr(buffer, bytesRead);
        HttpRequest request = parseRequest(requestStr);
        HttpResponse response = routeRequest(request);

        std::string responseStr = formatResponse(response);
        send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
        close(clientSocket);
    }

    HttpRequest parseRequest(const std::string& raw) {
        HttpRequest req;
        std::istringstream stream(raw);
        std::string line;

        std::getline(stream, line);
        std::istringstream requestLine(line);
        std::string methodStr, fullPath, httpVersion;
        requestLine >> methodStr >> fullPath >> httpVersion;

        if (methodStr == "GET") req.method = HttpMethod::GET;
        else if (methodStr == "POST") req.method = HttpMethod::POST;
        else if (methodStr == "PUT") req.method = HttpMethod::PUT;
        else if (methodStr == "DELETE") req.method = HttpMethod::DELETE;
        else if (methodStr == "OPTIONS") req.method = HttpMethod::OPTIONS;

        size_t queryPos = fullPath.find('?');
        if (queryPos != std::string::npos) {
            req.path = fullPath.substr(0, queryPos);
            std::string queryString = fullPath.substr(queryPos + 1);
            parseQueryParams(queryString, req.queryParams);
        } else {
            req.path = fullPath;
        }

        while (std::getline(stream, line) && line != "\r" && !line.empty()) {
            if (line.back() == '\r') line.pop_back();
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                req.headers[key] = value;
            }
        }

        std::string body;
        while (std::getline(stream, line)) {
            body += line;
        }
        if (!body.empty() && body.back() == '\r') body.pop_back();
        req.body = body;

        return req;
    }

    void parseQueryParams(const std::string& query, std::map<std::string, std::string>& params) {
        std::istringstream stream(query);
        std::string pair;
        while (std::getline(stream, pair, '&')) {
            size_t eqPos = pair.find('=');
            if (eqPos != std::string::npos) {
                params[pair.substr(0, eqPos)] = pair.substr(eqPos + 1);
            }
        }
    }

    HttpResponse routeRequest(HttpRequest& request) {
        if (request.method == HttpMethod::OPTIONS) {
            HttpResponse resp(204, "");
            resp.headers["Access-Control-Allow-Origin"] = "*";
            resp.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
            resp.headers["Access-Control-Allow-Headers"] = "Content-Type, X-User-Id";
            return resp;
        }

        for (auto& route : routes_) {
            if (route.method != request.method) continue;

            std::smatch match;
            if (std::regex_match(request.path, match, route.regex)) {
                for (size_t i = 0; i < route.paramNames.size() && i + 1 < match.size(); ++i) {
                    request.pathParams[route.paramNames[i]] = match[i + 1].str();
                }

                try {
                    HttpResponse resp = route.handler(request);
                    resp.headers["Access-Control-Allow-Origin"] = "*";
                    return resp;
                } catch (const std::exception& e) {
                    std::cerr << "Handler error: " << e.what() << std::endl;
                    return HttpResponse::internalError(e.what());
                }
            }
        }

        return HttpResponse::notFound("Route not found: " + request.path);
    }

    std::string formatResponse(const HttpResponse& response) {
        std::ostringstream ss;
        ss << "HTTP/1.1 " << response.statusCode << " ";

        switch (response.statusCode) {
            case 200: ss << "OK"; break;
            case 201: ss << "Created"; break;
            case 204: ss << "No Content"; break;
            case 400: ss << "Bad Request"; break;
            case 404: ss << "Not Found"; break;
            case 409: ss << "Conflict"; break;
            case 500: ss << "Internal Server Error"; break;
            default: ss << "Unknown"; break;
        }
        ss << "\r\n";

        for (const auto& [key, value] : response.headers) {
            ss << key << ": " << value << "\r\n";
        }
        ss << "Content-Length: " << response.body.length() << "\r\n";
        ss << "\r\n";
        ss << response.body;

        return ss.str();
    }
};

HttpServer::HttpServer(int port) : impl_(std::make_unique<Impl>(port)) {}

HttpServer::~HttpServer() = default;

void HttpServer::get(const std::string& path, RouteHandler handler) {
    impl_->addRoute(HttpMethod::GET, path, handler);
}

void HttpServer::post(const std::string& path, RouteHandler handler) {
    impl_->addRoute(HttpMethod::POST, path, handler);
}

void HttpServer::put(const std::string& path, RouteHandler handler) {
    impl_->addRoute(HttpMethod::PUT, path, handler);
}

void HttpServer::del(const std::string& path, RouteHandler handler) {
    impl_->addRoute(HttpMethod::DELETE, path, handler);
}

void HttpServer::start() {
    impl_->start();
}

void HttpServer::stop() {
    impl_->stop();
}

bool HttpServer::isRunning() const {
    return impl_->running_;
}

} // namespace hozon
