#pragma once

#include <array>
#include <functional>
#include <sstream>
#include <string>

/* Still WIP, interface subject to change */
namespace LibOpenNFS {
    enum class LogLevel {
        INFO,
        WARNING,
        DEBUG,
        COUNT
    };

    inline std::string get_string(const LogLevel level) {
        switch (level) {
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

    inline std::array<std::function<void(const char *, int, const char *, std::string)>, static_cast<size_t>(
        LogLevel::COUNT)> loggerFunctions;

    inline void RegisterLogCallback(const LogLevel logLevel,
                                    std::function<void(const char *, int, const char *, std::string)> callback) {
        loggerFunctions.at(static_cast<size_t>(logLevel)) = std::move(callback);
    };
} // namespace LibOpenNFS
