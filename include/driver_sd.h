#ifndef DRIVER_SD_H
#define DRIVER_SD_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "layout.h"
#include "cs_manager.h"


uint8_t sd_receive_byte() {
    cs_select_sd();
    uint8_t data = 0xFF;
    spi_read_blocking(SPI_PORT, 0xFF, &data, 1);
    return data;
}

void sd_send_byte(uint8_t data) {
    cs_select_sd();
    spi_write_blocking(SPI_PORT, &data, 1);
}

uint8_t sd_send_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t buffer[6] = {
        (uint8_t)(cmd | 0x40),
        (uint8_t)((arg >> 24) & 0xFF),
        (uint8_t)((arg >> 16) & 0xFF),
        (uint8_t)((arg >> 8) & 0xFF),
        (uint8_t)(arg & 0xFF),
        crc
    };

    cs_select_sd();
    spi_write_blocking(SPI_PORT, buffer, 6);

    uint8_t response;
    for (int i = 0; i < 10; i++) {
        response = sd_receive_byte();
        if (!(response & 0x80)) break;
    }
    
    // stdio_printf("CMD%u envoyé (arg: 0x%08X) -> Réponse: 0x%02X\n", cmd, arg, response);
    return response;
}

bool sd_read_block(uint32_t block, uint8_t *buffer) {
    uint8_t response;


    cs_select_sd();
    spi_init(SPI_PORT, SPI_SD_READ_BAUDRATE);
    gpio_put(PIN_LED, 1);

    // stdio_printf("\t> Lecture du bloc %lu...\n", block);
    
    response = sd_send_command(17, block, 0);
    if (response != 0x00) {
        stdio_printf("\t  [ERROR] CMD17 échoué (réponse : 0x%02X)\n", response);
        cs_no_select();
        spi_init(SPI_PORT, SPI_BAUDRATE);
        gpio_put(PIN_LED, 0);
        return false;
    }

    // stdio_printf("\t  Début de l'attente du token 0xFE...\n");
    for (int i = 0; i < 5000; i++) {
        response = sd_receive_byte();
        if (response == 0xFE) break;
        sleep_ms(1);
    }
    if (response != 0xFE) {
        stdio_printf("\t  [ERROR] Aucun token reçu, lecture annulée\n");
        cs_no_select();
        spi_init(SPI_PORT, SPI_BAUDRATE);
        gpio_put(PIN_LED, 0);
        return false;
    }

    // stdio_printf("\t  Token reçu, lecture des données...\n");
    spi_read_blocking(SPI_PORT, 0xFF, buffer, 512);
    uint8_t crc[2];
    spi_read_blocking(SPI_PORT, 0xFF, crc, 2);
    
    // stdio_printf("\t  Lecture terminée avec succès !\n");
    cs_no_select();
    spi_init(SPI_PORT, SPI_BAUDRATE);
    gpio_put(PIN_LED, 0);
    return true;
}

bool sd_init() {
    uint8_t response;

    spi_init(SPI_PORT, SPI_SD_INIT_BAUDRATE);  // basse fréquence pour l'initialisation

    // Désactiver CS et envoyer 80 cycles d'horloge pour réveiller la carte
    stdio_printf("Réveil de la carte SD...\n");
    cs_no_select();
    for (int i = 0; i < 10; i++) {
        sd_send_byte(0xFF);
    }
    sleep_ms(10);
    
    // Activer CS pour la communication SPI
    cs_select_sd();
    
    // Envoyer CMD0 plusieurs fois pour forcer le mode SPI
    stdio_printf("Passage en mode SPI...\n");
    for (int i = 0; i < 5; i++) {
        response = sd_send_command(0, 0, 0x95);
        stdio_printf("Réponse CMD0 : 0x%02X\n", response);
        if (response == 0x01) break;
        sleep_ms(50);
    }
    
    if (response != 0x01) {
        stdio_printf("[ERROR] CMD0 échoué, carte non détectée\n");
        cs_no_select();
        return false;
    }

    // Envoyer CMD8 pour détecter SDHC
    stdio_printf("Envoi de CMD8...\n");
    response = sd_send_command(8, 0x1AA, 0x87);
    if (response == 0x01) {
        stdio_printf("Carte SDHC détectée (CMD8 réponse : 0x01)\n");
    } else if (response & 0x04) {
        stdio_printf("CMD8 échoué, carte SD probablement ancienne\n");
    } else {
        stdio_printf("[ERROR] Réponse inattendue à CMD8 : 0x%02X\n", response);
        return false;
    }

    // Envoyer ACMD41 jusqu'à ce que la carte soit prête
    stdio_printf("Initialisation avec ACMD41...\n");
    for (int i = 0; i < 1000; i++) {
        sd_send_command(55, 0, 0x65);
        response = sd_send_command(41, 1 << 30, 0x77);

        stdio_printf("Réponse ACMD41 : 0x%02X\n", response);
        if (response == 0x00) {
            stdio_printf("Passage des blocs sur 512 octets\n");
            sd_send_command(16, 512, 0); // Fixer la taille des blocs
            stdio_printf("Carte SD prête !\n");
            spi_init(SPI_PORT, SPI_BAUDRATE);  // retour à une haute fréquence
            cs_no_select();
            return true;
        }
        sleep_ms(5);
    }

    stdio_printf("\n[ERROR] ACMD41 a échoué, carte non initialisée\n");
    cs_no_select();

    spi_init(SPI_PORT, SPI_BAUDRATE);  // retour à une haute fréquence

    return false;
}

#endif