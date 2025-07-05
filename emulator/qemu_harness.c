#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>
#include "os_api.h"

/*
USAGE: qemu-arm ./qemu_harness path_to_game.bin
COMPILATION : make -f Makefile.qemu
*/

// COMMUNICATION FORMAT : CMD:arg1,arg2,...

void setup_io() {  // Disable stdout and stderr buffering to have instantaneous communication between C and Python
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);
}

#pragma region OS_API_FUNCTIONS
// DEBUG / HW
// QEMU will redirect the printf to the user shell
int emu_print_serial(const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[API_CALL] print_serial: ");
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

void emu_backlight_state(char state) {
    printf("API:backlight:%d\n", state);
}

// GRAPHICS
void emu_flush_render_buffer(void) {
    printf("API:flush\n");
}

void emu_set_px(const uint16_t x_pos, const uint16_t y_pos, const uint16_t color) {
    printf("API:set_px:%u,%u,%u\n", x_pos, y_pos, color);
}

uint16_t emu_get_px(const uint16_t x_pos, const uint16_t y_pos) {
    printf("API:get_px:%u,%u)\n", x_pos, y_pos);

    char response_buffer[16];
    if (fgets(response_buffer, sizeof(response_buffer), stdin) != NULL) {
        uint16_t color = (uint16_t)strtol(response_buffer, NULL, 10);
        return color;
    }
    return 0;
}

void emu_put_sprite(const uint16_t x_pos, const uint16_t y_pos, const uint16_t width, const uint16_t height, const uint16_t *sprite) {
    printf("[DBG API_CALL] put_sprite(x: %u, y: %u, w: %u, h: %u, sprite_ptr: %p)\n", x_pos, y_pos, width, height, sprite);
}

// INPUTS
char emu_get_btn(const uint32_t button) {
    printf("API:get_btn:%u\n", button);

    char response_buffer[16];
    if (fgets(response_buffer, sizeof(response_buffer), stdin) != NULL) {
        if (strncmp(response_buffer, "1", 1) == 0) {
            return 1;
        }
    }
    return 0;
}

// MISC
uint32_t emu_get_rand_32(void) {
    printf("API:get_rand_32\n");
    return rand();
}

#pragma endregion


// Emulator entry point, which will be ran by QEMU
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_game.bin>\n", argv[0]);
        return 1;
    }
    const char* game_path = argv[1];
    FILE* game_file = NULL;
    void* game_code_buffer = NULL;
    long file_size = 0;

    setup_io();  // to disable buffering

    printf("[ QEMU ARM Harness - Asoboo ]\n");

    // OS api table definition, function assignement
    os_api_t emu_api_table = {
        .print_serial = emu_print_serial,
        .backlight_state = emu_backlight_state,
        .flush_render_buffer = emu_flush_render_buffer,
        .set_px = emu_set_px,
        .get_px = emu_get_px,
        .put_sprite = emu_put_sprite,
        .get_btn = emu_get_btn,
        .get_rand_32 = emu_get_rand_32,
    };
    
    // Open binary file
    game_file = fopen(game_path, "rb");
    if (!game_file) {
        perror("ERROR: Unable to open the game file");
        return 1;
    }

    fseek(game_file, 0, SEEK_END);
    file_size = ftell(game_file);
    fseek(game_file, 0, SEEK_SET);

    if (file_size <= 0) {
        fprintf(stderr, "ERROR: Empty or invalid game file\n");
        fclose(game_file);
        return 1;
    }
    printf("Reading '%s' (%ld bytes)...\n", game_path, file_size);

    // Allow executable memory for the game code
    // malloc allows non-executable memory, so we need to use mmap
    game_code_buffer = mmap(
        NULL, // The system choose the adress
        file_size, // Region size
        PROT_READ | PROT_WRITE | PROT_EXEC, // Permissions: R W X
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0
    );

    if (game_code_buffer == MAP_FAILED) {
        perror("ERROR: Unable to allow memory\n");
        fclose(game_file);
        return 1;
    }

    // Copy game binary into the executable memory area
    if (fread(game_code_buffer, 1, file_size, game_file) != file_size) {
        fprintf(stderr, "ERROR: unable to read the file into the buffer\n");
        fclose(game_file);
        munmap(game_code_buffer, file_size);
        return 1;
    }
    fclose(game_file);

    // Prepare game_start call
    printf("Game code loaded at %p\n", game_code_buffer);
    
    void (*game_main_ptr)(os_api_t*) = (void (*)(os_api_t*))((uintptr_t)game_code_buffer | 1);  // '| 1' needed because there is Thumb

    printf("Launching game_main at 0x%lx...\n\n", (uintptr_t)game_main_ptr);
    printf("CTL:ready\n");
    

    game_main_ptr(&emu_api_table);

    printf("\n<<< game_main() returned. End of emulation.\n");
    printf("CTL:done\n");

    // Free memory
    munmap(game_code_buffer, file_size);

    return 0;
}