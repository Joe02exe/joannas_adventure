#pragma once

#include <string>
#include <spdlog/spdlog.h>

#define LOGGING_ENABLED true

class Logger {
public:
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

    template <typename... Args>
    static void info(fmt::format_string<Args...> fmt, Args&&... args) {
        if constexpr (LOGGING_ENABLED) {
            spdlog::info(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void warning(fmt::format_string<Args...> fmt, Args&&... args) {
        if constexpr (LOGGING_ENABLED) {
            spdlog::warn(fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void error(fmt::format_string<Args...> fmt, Args&&... args) {
        if constexpr (LOGGING_ENABLED) {
            spdlog::error(fmt, std::forward<Args>(args)...);
        }
    }

    static void init();
};
