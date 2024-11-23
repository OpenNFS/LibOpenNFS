#pragma once

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
    inline void _InternalLog(const LogLevel logLevel, const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(loggingBuffer, LOG_BUFFER_SIZE, fmt, args);
        va_end(args);
        loggerFunctions.at(logLevel)(loggingBuffer);
    }

    // TODO: These aren't quite right yet either....
    #define LogInfo(X, ...) { _InternalLog(LONFS_INFO, __VA_ARGS__); };
    #define LogWarning(X, ...) { _InternalLog(LONFS_WARNING, __VA_ARGS__); };
    #define LogDebug(X, ...) { _InternalLog(LONFS_DEBUG, __VA_ARGS__); };
} // namespace LibOpenNFS
