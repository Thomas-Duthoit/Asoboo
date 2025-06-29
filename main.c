#include "pico/stdlib.h"
#include "init_hardware.h"
#include "ff.h"


FATFS fs;
DIR dir;
FILINFO fno;


void handle_fatal_error();

int main() {
    stdio_init_all();
    init_hardware();

    stdio_printf("Asoboo start\n");

    FRESULT fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        stdio_printf("Failed to mount : %d\n", fr);
        handle_fatal_error();
    } else {
        stdio_printf("Card mounted ! Listing root:\n");

        fr = f_opendir(&dir, "/");
        if (fr == FR_OK) {
            while (1) {
                fr = f_readdir(&dir, &fno);
                if (fr != FR_OK || fno.fname[0] == 0) break;
                if (fno.fattrib & AM_DIR)
                    stdio_printf("[DIR]  %s\n", fno.fname);
                else
                    stdio_printf("[FILE] %s (%lu bytes)\n", fno.fname, fno.fsize);
            }
            f_closedir(&dir);
        } else {
            stdio_printf("Failed to open dir \"/\": %d\n", fr);
            handle_fatal_error();
        }
    }

    

    while (true) {
        tight_loop_contents();
    }
}

void handle_fatal_error() {
    stdio_printf(">>> FATAL ERROR");
    // TODO: add something like a reboot
}