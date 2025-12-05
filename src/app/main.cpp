#include "joanna/core/game.h"

#include "joanna/utils/logger.h"

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

    Game::run();
    return 0;
}
