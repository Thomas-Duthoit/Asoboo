#include "init_hardware.h"

int main() {
    stdio_init_all();
    
    init_hardware();

    while (1) {
        tight_loop_contents();
    }
    
    return 0;
}
