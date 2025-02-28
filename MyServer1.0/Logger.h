#pragma once
#include <mutex>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdarg>

enum LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void logMessage(LogLevel level, const char* format, ...) {
        static std::mutex logMutex; // 静态互斥锁，保证全局唯一
        std::lock_guard<std::mutex> guard(logMutex); // 使用 lock_guard 自动管理互斥锁

        std::ofstream logFile("server.log", std::ios::app);
        
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        
        std::string levelStr;
        switch (level) {
            case INFO: levelStr = "INFO"; break;
            case WARNING: levelStr = "WARNING"; break;
            case ERROR: levelStr = "ERROR"; break;
        }

        va_list args;
        va_start(args, format);
        char buffer[2048];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        logFile << std::ctime(&now_c) << " [" << levelStr << "] " << buffer << std::endl;
        
        logFile.close();
    }
};

#define LOG_INFO(...) Logger::logMessage(INFO, __VA_ARGS__)
#define LOG_WARNING(...) Logger::logMessage(WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::logMessage(ERROR, __VA_ARGS__)
