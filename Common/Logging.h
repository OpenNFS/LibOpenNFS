#pragma once

#include <LibOpenNFS.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdarg>

namespace LibOpenNFS {
    constexpr uint32_t LOG_BUFFER_SIZE = 512;
    inline char loggingBuffer[LOG_BUFFER_SIZE];

    inline void _InternalLog(const LogLevel logLevel, const char *file, const int line, const char *func,
                             const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(loggingBuffer, LOG_BUFFER_SIZE, fmt, args);
        va_end(args);
        if (const auto &loggerFunction{loggerFunctions.at(static_cast<size_t>(logLevel))}) {
            loggerFunction(file, line, func, loggingBuffer);
        } else {
            // Register a fallback handler for the level
            std::cerr << "No " << get_string(logLevel) <<
                    " Logging callback provided to LibOpenNFS. Falling back to std." << std::endl;
            const std::function fallbackLoggerFunction =
                    [logLevel](const char *_file, const int _line, const char *_func, std::string logMessage) {
                auto &oss{logLevel == LogLevel::WARNING ? std::cerr : std::cout};
                const std::string file_str{_file};
                const std::string filename{file_str.substr(file_str.find_last_of("/\\") + 1)};
                const auto t{std::time(nullptr)};
                const auto tm{*std::localtime(&t)};
                oss << std::put_time(&tm, "%H:%M:%S") << " " << get_string(logLevel) << " [" << filename << "->" <<
                        _func <<
                        ":" << _line << "]: " << loggingBuffer << std::endl;
            };
            loggerFunctions[static_cast<size_t>(logLevel)] = fallbackLoggerFunction;
            // Then call it. On Subsequent invocations, we can just look it up.
            fallbackLoggerFunction(file, line, func, loggingBuffer);
        }
    }

#define LogInfo(...) { _InternalLog(LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
#define LogWarning(...) { _InternalLog(LogLevel::WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
#define LogDebug(...) { _InternalLog(LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
}
