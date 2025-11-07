#include "game.h"

#include "spdlog/spdlog.h"

int main() {

#if LOGGING_ENABLED
    spdlog::info("Logging enabled");
#endif

#if GOD_MODE_ENABLED
    spdlog::info("God mode enabled");
#endif

#if INFINITE_RESOURCES_ENABLED
    spdlog::info("Infinite resources enabled");
#endif

    Game game;
    game.run();
    return 0;
}
