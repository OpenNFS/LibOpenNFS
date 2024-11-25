#pragma once

#include <array>
#include <functional>
#include <sstream>
#include <string>
#include <ctime>

/* Still WIP, interface subject to change */
namespace LibOpenNFS {
    enum class LogLevel {
        INFO,
        WARNING,
        DEBUG,
        COUNT
    };

    inline std::string get_string(LogLevel level) {
       switch (level){
           case LogLevel::INFO:
               return "INFO";
           case LogLevel::WARNING:
               return "WARNING";
           case LogLevel::DEBUG:
               return "DEBUG";
           default:
               return "";
       }
    }

    constexpr uint32_t LOG_BUFFER_SIZE = 512;
    inline char loggingBuffer[LOG_BUFFER_SIZE];

    inline std::array<std::function<void(const char *, int, const char *, std::string)>, static_cast<size_t>(
        LogLevel::COUNT)> loggerFunctions;

    inline void RegisterLogCallback(const LogLevel logLevel,
                                    std::function<void(const char *, int, const char *, std::string)> callback) {
        loggerFunctions.at(static_cast<size_t>(logLevel)) = std::move(callback);
    };

    inline void _InternalLog(const LogLevel logLevel, const char *file, const int line, const char *func,
                             const char *fmt, ...) {
        static bool first {false};
        va_list args;
        va_start(args, fmt);
        vsnprintf(loggingBuffer, LOG_BUFFER_SIZE, fmt, args);
        va_end(args);
        if (const auto &loggerFunction {loggerFunctions.at(static_cast<size_t>(logLevel))}) {
            loggerFunction(file, line, func, loggingBuffer);
        } else {
            if (!first) {
                first = true;
                std::cerr << "No Logging callback provided to LibOpenNFS. Falling back to std" << std::endl;
            }
            auto &oss {logLevel == LogLevel::WARNING ? std::cerr : std::cout};
            const std::string file_str {file};
            const std::string filename {file_str.substr(file_str.find_last_of("/\\") + 1)};
            const auto t {std::time(nullptr)};
            const auto tm {*std::localtime(&t)};
            oss << std::put_time(&tm, "%H:%M:%S") << " " << get_string(logLevel) << " [" << filename << "->" << func << ":" << line << "]: " << loggingBuffer << std::endl;
        }
    }

#define LogInfo(...) { _InternalLog(LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
#define LogWarning(...) { _InternalLog(LogLevel::WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
#define LogDebug(...) { _InternalLog(LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
} // namespace LibOpenNFS
