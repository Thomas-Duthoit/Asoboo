#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "os_api.h"
#include "init_hardware.h"
#include "ff.h"


#define GAME_CODE_ADDR     0x20030000
#define GAME_CODE_MAX_SIZE 0x00010000  // 64 Ko

os_api_t os_api_table = {
    .print = stdio_printf,
    .bl = screen_backlight
};


void load_game(const char* path) {
    UINT br;
    FIL file;
    void (*game_main)(os_api_t*);
    uint32_t saved_sp;


    stdio_printf("opening and loading \"%s\" ...\n", path);
    f_open(&file, path, FA_READ);
    f_read(&file, (void *)GAME_CODE_ADDR, GAME_CODE_MAX_SIZE, &br);
    f_close(&file);

    stdio_printf("Loaded %d bytes\n", br);
    
    // For Thumb (which is needed for the pico), add 1 to the address
    game_main = (void (*)(os_api_t *))(GAME_CODE_ADDR | 1);

    // Save current stack pointer
    __asm volatile ("mov %0, sp" : "=r"(saved_sp));
    // Flush cache
    __asm volatile ("dsb");
    __asm volatile ("isb");
    
    stdio_printf("... launching at 0x%08x!\n", (uint32_t)game_main);
    
    
    // Move the stack pointer
    __asm volatile ("mov sp, %0" :: "r"(GAME_CODE_ADDR + GAME_CODE_MAX_SIZE - 4));
    
    game_main(&os_api_table);

    // Restore original stack pointer
    __asm volatile ("mov sp, %0" :: "r"(saved_sp));
    
    stdio_printf("... exec done !\n");
}

#endif // BOOTLOADER_H
