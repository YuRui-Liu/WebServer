#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Database.h"

// Router 类负责将特定的 HTTP 请求映射到相应的处理函数
class Router {
public:
    // 定义处理函数的类型
    using HandlerFunc = std::function<HttpResponse(const HttpRequest&)>;

    // 添加路由：将 HTTP 方法和路径映射到处理函数
    void addRoute(const std::string& method, const std::string& path, HandlerFunc handler) {
        routes[method + "|" + path] = handler;
    }

    // 根据 HTTP 请求路由到相应的处理函数
    HttpResponse routeRequest(const HttpRequest& request) {
        std::string key = request.getMethodString() + "|" + request.getPath();
        if (routes.count(key)) {
            return routes[key](request);
        }
        // 如果没有找到匹配的路由，返回 404 Not Found 响应
        return HttpResponse::makeErrorResponse(404, "Not Found");
    }


    std::string readFile(const std::string& filePath) {
        // 使用标准库中的ifstream打开文件
        std::ifstream file(filePath);

        // 判断文件是否成功打开
        if (!file.is_open()) {
            // 若未能成功打开文件，返回错误信息
            return "Error: Unable to open file " + filePath;
        }

        // 使用stringstream来读取文件内容
        std::stringstream buffer;
        // 将文件内容读入到stringstream中
        buffer << file.rdbuf();

        // 将读取的内容转换为字符串并返回
        return buffer.str();
    }

    // 设置数据库相关的路由，例如注册和登录
    void setupDatabaseRoutes(Database& db) {

        addRoute("GET", "/login", [this](const HttpRequest& req) {
            HttpResponse response;
            response.setStatusCode(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(readFile("UI/login.html"));
            return response;
        });

        addRoute("GET", "/register", [this](const HttpRequest& req) {
            HttpResponse response;
            response.setStatusCode(200);
            response.setHeader("Content-Type", "text/html");
            response.setBody(readFile("UI/register.html"));
            return response;
        });

        // 注册路由
        addRoute("POST", "/register", [&db](const HttpRequest& req) {
            auto params = req.parseFormBody();  // 解析表单数据
            std::string username = params["username"];
            std::string password = params["password"];
            // 调用数据库方法进行用户注册
            if (db.registerUser(username, password)) {
                // return HttpResponse::makeOkResponse("Register Success!");
                
                // HttpResponse response;
                // response.setStatusCode(302); // 设置状态码为302，表示重定向
                // response.setHeader("Location", "/login"); // 设置Location头字段为登录页面的URL
                // return response; // 返回重定向响应

                HttpResponse response;
                response.setStatusCode(200); // HTTP 状态码 200 表示成功
                response.setHeader("Content-Type", "text/html");
                std::string responseBody = R"(
                    <html>
                    <head>
                        <title>Register Success</title>
                        <script type="text/javascript">
                            alert("Register Success!");
                            window.location = "/login";
                        </script>
                    </head>
                    <body>
                        <h2>moving to login...</h2>
                    </body>
                    </html>
                )";
                response.setBody(responseBody);
                return response;
            } else {
                return HttpResponse::makeErrorResponse(400, "Register Failed!");
            }
        });

        // 登录路由
        addRoute("POST", "/login", [&db](const HttpRequest& req) {
            auto params = req.parseFormBody();  // 解析表单数据
            std::string username = params["username"];
            std::string password = params["password"];
            LOG_INFO("Start check user info");
            // 调用数据库方法进行用户登录
            if (db.loginUser(username, password)) {
                    HttpResponse response;
                    response.setStatusCode(200); // HTTP 状态码 200 表示成功
                    response.setHeader("Content-Type", "text/html");
                    response.setBody("<html><body><h2>Login Successful</h2></body></html>");
                    LOG_INFO("Login Success!");
                    return response;
                } else {
                    // 登录失败
                    HttpResponse response;
                    response.setStatusCode(401); // HTTP 状态码 401 表示未授权
                    response.setHeader("Content-Type", "text/html");
                    response.setBody("<html><body><h2>Login Failed</h2></body></html>");
                    return response;
                }
        });
    }

private:
    std::unordered_map<std::string, HandlerFunc> routes;  // 存储路由映射
};
