#include "joanna/utils/logger.h"

void Logger::init() {
    if constexpr (LOGGING_ENABLED) {
        // Set global log pattern and level
        spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
        spdlog::set_level(spdlog::level::info);
    }
}

void Logger::info(const std::string& message) {
    if constexpr (LOGGING_ENABLED) {
        spdlog::info(message);
    }
}

void Logger::warning(const std::string& message) {
    if constexpr (LOGGING_ENABLED) {
        spdlog::warn(message);
    }
}

void Logger::error(const std::string& message) {
    if constexpr (LOGGING_ENABLED) {
        spdlog::error(message);
    }
}
