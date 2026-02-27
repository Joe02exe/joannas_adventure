#include "joanna/core/game.h"

#include "joanna/utils/logger.h"
#include <exception>
#include <iostream>

int main() {

#if LOGGING_ENABLED
    Logger::info("Logging enabled");
#endif

#if GOD_MODE_ENABLED
    Logger::info("God mode enabled");
#endif

#if INFINITE_RESOURCES_ENABLED
    Logger::info("Infinite resources enabled");
#endif

    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        fprintf(stderr, "FATAL ERROR: %s\n", e.what());
        return 1;
    }
    return 0;
}
