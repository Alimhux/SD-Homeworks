#include "http_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <regex>

namespace hozon {

nlohmann::json HttpClientResponse::json() const {
    if (body.empty()) {
        return nlohmann::json::object();
    }
    try {
        return nlohmann::json::parse(body);
    } catch (...) {
        return nlohmann::json::object();
    }
}

HttpClient::HttpClient() {}

HttpClient::~HttpClient() {}

HttpClient::UrlParts HttpClient::parseUrl(const std::string& url) {
    UrlParts parts;
    parts.port = 80;
    parts.path = "/";

    std::regex urlRegex("^http://([^:/]+)(?::(\\d+))?(.*)$");
    std::smatch match;

    if (std::regex_match(url, match, urlRegex)) {
        parts.host = match[1].str();
        if (match[2].matched) {
            parts.port = std::stoi(match[2].str());
        }
        if (match[3].matched && !match[3].str().empty()) {
            parts.path = match[3].str();
        }
    } else {
        throw std::runtime_error("Invalid URL format: " + url);
    }

    return parts;
}

HttpClientResponse HttpClient::request(const std::string& method,
                                        const std::string& url,
                                        const std::string& body,
                                        const std::map<std::string, std::string>& headers) {
    UrlParts parts = parseUrl(url);

    // Разрешение имени хоста
    struct hostent* host = gethostbyname(parts.host.c_str());
    if (!host) {
        throw std::runtime_error("Could not resolve host: " + parts.host);
    }

    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Установка таймаута
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // Подключение
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(parts.port);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        throw std::runtime_error("Failed to connect to " + parts.host + ":" + std::to_string(parts.port));
    }

    // Формирование запроса
    std::ostringstream request;
    request << method << " " << parts.path << " HTTP/1.1\r\n";
    request << "Host: " << parts.host << "\r\n";
    request << "Connection: close\r\n";

    for (const auto& [key, value] : headers) {
        request << key << ": " << value << "\r\n";
    }

    if (!body.empty()) {
        request << "Content-Length: " << body.length() << "\r\n";
        if (headers.find("Content-Type") == headers.end()) {
            request << "Content-Type: application/json\r\n";
        }
    }

    request << "\r\n";
    request << body;

    std::string requestStr = request.str();

    // Отправка запроса
    if (send(sock, requestStr.c_str(), requestStr.length(), 0) < 0) {
        close(sock);
        throw std::runtime_error("Failed to send request");
    }

    // Получение ответа
    std::string responseStr;
    char buffer[4096];
    ssize_t bytesRead;

    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        responseStr += buffer;
    }

    close(sock);

    // Парсинг ответа
    HttpClientResponse response;
    response.statusCode = 0;

    std::istringstream stream(responseStr);
    std::string line;

    // Парсинг строки статуса
    if (std::getline(stream, line)) {
        std::istringstream statusLine(line);
        std::string httpVersion;
        statusLine >> httpVersion >> response.statusCode;
    }

    // Парсинг заголовков
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        if (line.back() == '\r') line.pop_back();
        if (line.empty()) break;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            response.headers[key] = value;
        }
    }

    // Парсинг тела (остаток ответа)
    std::ostringstream bodyStream;
    while (std::getline(stream, line)) {
        bodyStream << line;
    }
    response.body = bodyStream.str();

    // Обработка chunked-кодирования (упрощённая)
    if (response.headers.count("Transfer-Encoding") &&
        response.headers["Transfer-Encoding"].find("chunked") != std::string::npos) {
        // Простое декодирование - извлечение контента между маркерами чанков
        std::string dechunked;
        std::istringstream chunkStream(response.body);
        std::string chunkLine;

        while (std::getline(chunkStream, chunkLine)) {
            if (chunkLine.back() == '\r') chunkLine.pop_back();
            // Пропуск строк с размером чанка (hex-числа)
            if (chunkLine.empty()) continue;
            bool isChunkSize = true;
            for (char c : chunkLine) {
                if (!std::isxdigit(c)) {
                    isChunkSize = false;
                    break;
                }
            }
            if (!isChunkSize) {
                dechunked += chunkLine;
            }
        }
        response.body = dechunked;
    }

    return response;
}

HttpClientResponse HttpClient::get(const std::string& url,
                                   const std::map<std::string, std::string>& headers) {
    return request("GET", url, "", headers);
}

HttpClientResponse HttpClient::post(const std::string& url,
                                    const std::string& body,
                                    const std::map<std::string, std::string>& headers) {
    return request("POST", url, body, headers);
}

HttpClientResponse HttpClient::put(const std::string& url,
                                   const std::string& body,
                                   const std::map<std::string, std::string>& headers) {
    return request("PUT", url, body, headers);
}

HttpClientResponse HttpClient::del(const std::string& url,
                                   const std::map<std::string, std::string>& headers) {
    return request("DELETE", url, "", headers);
}

} // namespace hozon
