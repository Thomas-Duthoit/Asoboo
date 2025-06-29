// #include "init_hardware.h"

// int main() {
//     stdio_init_all();
    
//     init_hardware();

//     while (1) {
//         tight_loop_contents();
//     }
    
//     return 0;
// }


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"  // FatFS

// === CONFIGURATION SPI SD ===
#define SPI_PORT spi0
#define PIN_MISO      16
#define PIN_MOSI      19
#define PIN_SCK       18
#define PIN_CS_SD 26

FATFS fs;
FILINFO fno;
DIR dir;

// === Initialisation SPI SD ===
void sd_spi_init(void) {
    spi_init(SPI_PORT, 1 * 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_SD);
    gpio_set_dir(PIN_CS_SD, GPIO_OUT);
    gpio_put(PIN_CS_SD, 1);  // désélection

    printf("SPI SD initialisé.\n");
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // attendre que le terminal s'ouvre
    printf("=== Démarrage ===\n");

    sd_spi_init();

    FRESULT fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("Erreur montage carte SD : %d\n", fr);
        return 1;
    }

    printf("Carte SD montée avec succès !\n");
    printf("Contenu de la racine :\n");

    fr = f_opendir(&dir, "/");
    if (fr == FR_OK) {
        while (1) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;  // fin
            if (fno.fattrib & AM_DIR) {
                printf("   <DIR>  %s\n", fno.fname);
            } else {
                printf("   <FILE> %s (%lu octets)\n", fno.fname, fno.fsize);
            }
        }
        f_closedir(&dir);
    } else {
        printf("Erreur ouverture répertoire : %d\n", fr);
    }

    while (1) {
        tight_loop_contents();
    }
}