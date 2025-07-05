// --- START OF FILE qemu_harness.c ---

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h> // Pour mmap, afin d'allouer de la mémoire exécutable
#include "os_api.h"

// =================================================================
// Implémentations simulées ("mock") des fonctions de l'API de l'OS
// Ces fonctions sont appelées par le code ARM de game.bin
// =================================================================

// DEBUG / HW
// QEMU redirigera ce printf vers la console du PC hôte
int emu_print_serial(const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[API_CALL] print_serial: ");
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

void emu_backlight_state(char state) {
    printf("[API_CALL] backlight_state(state: %d)\n", state);
}

// GRAPHICS
void emu_flush_render_buffer(void) {
    printf("[API_CALL] flush_render_buffer()\n");
}

void emu_set_px(const uint16_t x_pos, const uint16_t y_pos, const uint16_t color) {
    printf("[API_CALL] set_px(x: %u, y: %u, color: 0x%04X)\n", x_pos, y_pos, color);
}

uint16_t emu_get_px(const uint16_t x_pos, const uint16_t y_pos) {
    printf("[API_CALL] get_px(x: %u, y: %u)\n", x_pos, y_pos);
    return 0;
}

void emu_put_sprite(const uint16_t x_pos, const uint16_t y_pos, const uint16_t width, const uint16_t height, const uint16_t *sprite) {
    printf("[API_CALL] put_sprite(x: %u, y: %u, w: %u, h: %u, sprite_ptr: %p)\n", x_pos, y_pos, width, height, sprite);
}

// INPUTS
char emu_get_btn(const uint32_t button) {
    printf("[API_CALL] get_btn(button_pin: %u)\n", button);
    return 0; // Toujours non pressé
}

// MISC
uint32_t emu_get_rand_32(void) {
    printf("[API_CALL] get_rand_32()\n");
    return rand();
}

// =================================================================
// Point d'entrée du harness (sera lancé par qemu-arm)
// =================================================================

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_game.bin>\n", argv[0]);
        return 1;
    }
    const char* game_path = argv[1];
    FILE* game_file = NULL;
    void* game_code_buffer = NULL;
    long file_size = 0;

    printf("--- QEMU ARM Harness pour Asoboo ---\n");

    // Définition de la table d'API avec nos fonctions simulées
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
    
    // 1. Ouvrir et lire le binaire du jeu
    game_file = fopen(game_path, "rb");
    if (!game_file) {
        perror("Erreur: Impossible d'ouvrir le fichier du jeu");
        return 1;
    }

    fseek(game_file, 0, SEEK_END);
    file_size = ftell(game_file);
    fseek(game_file, 0, SEEK_SET);

    if (file_size <= 0) {
        fprintf(stderr, "Erreur: Fichier de jeu vide ou invalide.\n");
        fclose(game_file);
        return 1;
    }
    printf("Lecture de '%s' (%ld bytes)...\n", game_path, file_size);

    // 2. Allouer de la mémoire EXÉCUTABLE pour le code du jeu
    // C'est crucial. malloc() alloue de la mémoire non-exécutable par sécurité (NX bit).
    // Nous devons utiliser mmap() pour demander explicitement la permission d'exécuter.
    game_code_buffer = mmap(
        NULL, // Laisse le système choisir l'adresse
        file_size, // Taille de la région
        PROT_READ | PROT_WRITE | PROT_EXEC, // Permissions: lecture, écriture, EXÉCUTION
        MAP_PRIVATE | MAP_ANONYMOUS, // Mapping privé et anonyme
        -1, 0
    );

    if (game_code_buffer == MAP_FAILED) {
        perror("Erreur: Impossible d'allouer de la mémoire exécutable avec mmap");
        fclose(game_file);
        return 1;
    }

    // 3. Copier le code du jeu dans le buffer exécutable
    if (fread(game_code_buffer, 1, file_size, game_file) != file_size) {
        fprintf(stderr, "Erreur lors de la lecture du fichier dans le buffer.\n");
        fclose(game_file);
        munmap(game_code_buffer, file_size);
        return 1;
    }
    fclose(game_file);

    // 4. Préparer l'appel au jeu
    printf("Code du jeu chargé à l'adresse mémoire %p\n", game_code_buffer);
    
    // Création d'un pointeur de fonction vers le début du code du jeu.
    // L'ARM Cortex-M0+ du Pico utilise le jeu d'instructions Thumb.
    // Pour indiquer un saut en mode Thumb, l'adresse doit avoir son bit 0 à 1.
    void (*game_main_ptr)(os_api_t*) = (void (*)(os_api_t*))((uintptr_t)game_code_buffer | 1);

    printf("Lancement de game_main à l'adresse 0x%lx...\n\n", (uintptr_t)game_main_ptr);
    
    // 5. Appeler le jeu !
    // QEMU va maintenant commencer à exécuter les instructions ARM de game.bin.
    // L'argument &emu_api_table (passé via le registre R0 par le compilateur)
    // donnera au jeu l'accès à nos fonctions simulées.
    game_main_ptr(&emu_api_table);
    
    printf("\n<<< game_main() a retourné. Fin de l'émulation.\n");

    // Libérer la mémoire
    munmap(game_code_buffer, file_size);

    return 0;
}