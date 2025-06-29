#include "os_api.h"

__attribute__((used)) __attribute__((section(".text.game_main")))
void game_main(os_api_t* api) {
    api->bl(0);
    api->print("test n°%d\n", 0);
    return;
}