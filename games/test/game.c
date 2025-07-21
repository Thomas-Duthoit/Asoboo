#include "os_api.h"

__attribute__((used)) __attribute__((section(".text.game_main")))
void game_main(os_api_t* api) {
    api->backlight_state(0);
    api->print_serial("test n°%d\n", 1);
    // api->print_serial_int(1);
    // api->print_serial("\n");
    return;
}