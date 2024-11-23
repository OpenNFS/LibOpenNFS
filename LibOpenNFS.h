#pragma once

#include <array>
#include <functional>
#include <sstream>

/* Still WIP, interface subject to change */
namespace LibOpenNFS {
    enum LogLevel {
        LONFS_INFO,
        LONFS_WARNING,
        LONFS_DEBUG,
        COUNT
    };

    // TODO: Convert all of this over to be std::fmt based
    constexpr uint32_t LOG_BUFFER_SIZE = 256;
    inline char loggingBuffer[LOG_BUFFER_SIZE];

    // TODO: It would be much nicer if these were streams, like OpenNFS itself
    inline std::array<std::function<void(std::string)>, COUNT> loggerFunctions;

    inline void RegisterLogCallback(const LogLevel logLevel, std::function<void(std::string)> callback) {
        loggerFunctions.at(logLevel) = std::move(callback);
    };

    // I hate this
    inline void _InternalLog(const LogLevel logLevel, const char *file, uint32_t line, const char *func,
                             const char *fmt, ...) {
        // TODO: Do something with file, line, func
        va_list args;
        va_start(args, fmt);
        vsnprintf(loggingBuffer, LOG_BUFFER_SIZE, fmt, args);
        va_end(args);
        loggerFunctions.at(logLevel)(loggingBuffer);
    }

#define LogInfo(...) { _InternalLog(LONFS_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
#define LogWarning(...) { _InternalLog(LONFS_WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
#define LogDebug(...) { _InternalLog(LONFS_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); };
} // namespace LibOpenNFS
