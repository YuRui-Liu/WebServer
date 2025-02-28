// http_response.h
#pragma once
#include <string>
#include <unordered_map>
#include <sstream>

class HttpResponse {
public:
    HttpResponse(int code = 200) : statusCode(code) {}

    void setStatusCode(int code) {
        statusCode = code;
    }

    void setHeader(const std::string& name, const std::string& value) {
        headers[name] = value;
    }

    void setBody(const std::string& b) {
        body = b;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << statusCode << " " << getStatusMessage() << "\r\n";
        for (const auto& header : headers) {
            oss << header.first << ": " << header.second << "\r\n";
        }
        oss << "\r\n" << body;
        return oss.str();
    }

    static HttpResponse makeErrorResponse(int code, const std::string& message) {
        HttpResponse response(code);
        response.setBody(message);
        return response;
    }
    static HttpResponse makeOkResponse(const std::string& message) {
        HttpResponse response(200);
        response.setBody(message);
        return response;
    }

private:
    std::string getStatusMessage() const {
        switch (statusCode) {
            case 200: return "OK";
            case 404: return "Not Found";
            // ... 其他状态码 ...
            default: return "Unknown";
        }
    }

    int statusCode;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};
