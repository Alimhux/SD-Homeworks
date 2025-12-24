#ifndef HOZON_HTTP_CLIENT_H
#define HOZON_HTTP_CLIENT_H

#include <string>
#include <map>
#include "json.h"

namespace hozon {

struct HttpClientResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;

    nlohmann::json json() const;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // HTTP methods
    HttpClientResponse get(const std::string& url,
                          const std::map<std::string, std::string>& headers = {});

    HttpClientResponse post(const std::string& url,
                           const std::string& body,
                           const std::map<std::string, std::string>& headers = {});

    HttpClientResponse put(const std::string& url,
                          const std::string& body,
                          const std::map<std::string, std::string>& headers = {});

    HttpClientResponse del(const std::string& url,
                          const std::map<std::string, std::string>& headers = {});

private:
    HttpClientResponse request(const std::string& method,
                              const std::string& url,
                              const std::string& body,
                              const std::map<std::string, std::string>& headers);

    struct UrlParts {
        std::string host;
        int port;
        std::string path;
    };

    UrlParts parseUrl(const std::string& url);
};

} // namespace hozon

#endif // HOZON_HTTP_CLIENT_H
