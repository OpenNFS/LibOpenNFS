#pragma once

#include <array>
#include <functional>
#include <string>

/* Still WIP, interface subject to change */
namespace LibOpenNFS {
    enum class LogLevel {
        INFO,
        WARNING,
        DEBUG,
        COUNT
    };

    inline std::array<std::function<void(char const *, int, char const *, std::string)>,
                      static_cast<size_t>(LogLevel::COUNT)>
        loggerFunctions;

    inline void RegisterLogCallback(LogLevel const logLevel,
                                    std::function<void(char const *, int, char const *, std::string)> callback) {
        loggerFunctions.at(static_cast<size_t>(logLevel)) = std::move(callback);
    };
} // namespace LibOpenNFS
