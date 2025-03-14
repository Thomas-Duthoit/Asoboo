#include "os.h"

int main() {
    stdio_init_all();
    
    os_init();

    sleep_ms(1000);  // 1000 ms d'attente pour voir le boot screen

    os_main_menu();

    sleep_ms(1000);
    
    // os_run_game(GAMES.games[0]);

    // g_fill_rect(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, CYAN);

    while (1) {
        tight_loop_contents();
    }
    
    return 0;
}
