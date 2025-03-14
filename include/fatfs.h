#ifndef FATFS_H
#define FATFS_H

#include "driver_sd.h"
#include "string.h"


typedef uint8_t block_t[512];
typedef struct {
    uint32_t partition_start;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint32_t fat_start;
    uint32_t root_cluster;
    uint32_t data_start;
} fat_bs_t;  // FAT Boot Sector


bool         init_fatfs                                  ();
uint32_t    _fatfs_get_partition_start                   ();
fat_bs_t    _fatfs_read_boot_sector                      (uint32_t partition_start);
uint32_t    _fatfs_read_fat_entry                        (fat_bs_t *boot, uint32_t cluster);
void        _fatfs_list_root                             (fat_bs_t *boot);
void        _fatfs_format_short_name                     (char *filename);
uint32_t    _fatfs_cluster_to_sector_from_data_start     (fat_bs_t *boot, uint32_t cluster);
// bool        _fatfs_read_file                             (fat_bs_t *boot, uint32_t start_cluster, uint32_t file_size);


fat_bs_t BOOT = {0};



bool init_fatfs() {
    uint32_t fat_partition_addr;

    stdio_printf("-----------------------------------------\n");
    stdio_printf("Initialisation uSD\n");
    if (!sd_init()) {
        stdio_printf("[ERROR] sd_init\n");
        stdio_printf("-----------------------------------------\n");
        return false;
    }
    stdio_printf("Initialisation uSD réussie\n");

    stdio_printf("Lecture du MBR (bloc 0)\n");
    fat_partition_addr = _fatfs_get_partition_start();
    if (!fat_partition_addr) {
        stdio_printf("[ERROR] _fatfs_get_partition_start\n");
        stdio_printf("-----------------------------------------\n");
        return false;
    }

    stdio_printf("Lecture du Boot Sector\n");
    BOOT = _fatfs_read_boot_sector(fat_partition_addr);
    _fatfs_list_root(&BOOT);
    stdio_printf("-----------------------------------------\n");
    return true;
}

uint32_t _fatfs_get_partition_start() {
    block_t buffer;

    if (!sd_read_block(0, buffer)) {
        stdio_printf("[ERROR] Impossible de lire le MBR\n");
        return false;
    }

    if (buffer[510] != 0x55 || buffer[511] != 0xAA) {
        stdio_printf("[ERROR] Signature MBR invalide\n");
        return false;
    }

    uint32_t partition_start = buffer[0x1C6] | (buffer[0x1C7] << 8) | (buffer[0x1C8] << 16) | (buffer[0x1C9] << 24);
    stdio_printf("Partition FAT détectée à : %lu\n", partition_start);
    return partition_start;
}

fat_bs_t _fatfs_read_boot_sector(uint32_t partition_start) {
    block_t buffer;
    fat_bs_t boot = {0};

    if (!sd_read_block(partition_start, buffer)) {
        stdio_printf("[ERROR] Impossible de lire le Boot Sector\n");
        return boot;
    }

    boot.partition_start = partition_start;
    boot.bytes_per_sector = buffer[0x0B] | (buffer[0x0C] << 8);
    boot.sectors_per_cluster = buffer[0x0D];
    boot.fat_start = partition_start + (buffer[0x0E] | (buffer[0x0F] << 8));
    boot.root_cluster = buffer[0x2C] | (buffer[0x2D] << 8) | (buffer[0x2E] << 16) | (buffer[0x2F] << 24);
    uint32_t sectors_per_fat = buffer[0x24] | (buffer[0x25] << 8) | (buffer[0x26] << 16) | (buffer[0x27] << 24);
    boot.data_start = boot.fat_start + (sectors_per_fat * buffer[0x10]);

    stdio_printf("Boot Sector lu : %u octets/secteur, %u secteurs/cluster\n", boot.bytes_per_sector, boot.sectors_per_cluster);
    stdio_printf("FAT Start : %lu, Data Start : %lu, Root Cluster : %lu\n", boot.fat_start, boot.data_start, boot.root_cluster);
    return boot;
}

uint32_t _fatfs_read_fat_entry(fat_bs_t *boot, uint32_t cluster) {
    block_t buffer;
    uint32_t fat_sector = boot->fat_start + (cluster * 4) / boot->bytes_per_sector;
    uint16_t offset = (cluster * 4) % boot->bytes_per_sector;

    if (!sd_read_block(fat_sector, buffer)) {
        stdio_printf("[ERROR] Impossible de lire la FAT\n");
        return 0x0FFFFFFF;
    }

    return buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
}

void _fatfs_list_root(fat_bs_t *boot) {
    block_t buffer;
    uint32_t cluster = boot->root_cluster;

    uint32_t sector = _fatfs_cluster_to_sector_from_data_start(boot, cluster);

    stdio_printf("Lecture du Root Directory (Cluster %lu)\n", cluster);

    char long_filename[256] = {0};  // Stocke le nom long
    char filename[12] = {0};        // Stocke le nom court

    for (uint32_t i = 0; i < boot->sectors_per_cluster; i++) {
        if (!sd_read_block(sector + i, buffer)) {
            stdio_printf("[ERROR] Impossible de lire le Root Directory\n");
            return;
        }

        for (int j = 0; j < 512; j += 32) {
            if (buffer[j] == 0x00) {
                stdio_printf("Fin des entrées\n");
                return;
            }

            if (buffer[j] == 0xE5) continue;  // Fichier supprimé

            // Vérifier si c'est une entrée LFN (indicateur 0x0F dans l'attribut)
            if (buffer[j + 0x0B] == 0x0F) {
                int index = (buffer[j] & 0x1F) - 1;  // Récupérer l'index de la séquence LFN
                int pos = index * 13;

                for (int k = 0; k < 5; k++) long_filename[pos + k] = buffer[j + 1 + k * 2];
                for (int k = 0; k < 6; k++) long_filename[pos + 5 + k] = buffer[j + 14 + k * 2];
                for (int k = 0; k < 2; k++) long_filename[pos + 11 + k] = buffer[j + 28 + k * 2];

                continue;  // Passer à l'entrée suivante (entrée LFN)
            }

            uint8_t attr = buffer[j + 0x0B];
            uint16_t cluster_low = buffer[j + 0x1A] | (buffer[j + 0x1B] << 8);
            uint16_t cluster_high = buffer[j + 0x14] | (buffer[j + 0x15] << 8);
            uint32_t file_cluster = (cluster_high << 16) | cluster_low;
            uint32_t size = buffer[j + 0x1C] | (buffer[j + 0x1D] << 8) | 
                            (buffer[j + 0x1E] << 16) | (buffer[j + 0x1F] << 24);

            // Si une entrée LFN a été lue, utiliser son nom


            if (attr & 0x10) {  // DOSSIER
                if (long_filename[0]) {
                    stdio_printf("%s/\n", long_filename);
                    memset(long_filename, 0, sizeof(long_filename));  // Réinitialiser après affichage
                } else {
                    memcpy(filename, &buffer[j], 8);                   
                    _fatfs_format_short_name(filename);  // Nettoyer les espaces inutiles
                    stdio_printf("%s/\n", filename);
                }
            } else {  // FICHIER
                if (long_filename[0]) {
                    stdio_printf("Fichier : %s", long_filename);
                    memset(long_filename, 0, sizeof(long_filename));  // Réinitialiser après affichage
                } else {
                    memcpy(filename, &buffer[j], 8);
                    filename[8] = '.';
                    memcpy(&filename[9], &buffer[j + 8], 3);
                    
                    _fatfs_format_short_name(filename);  // Nettoyer les espaces inutiles
                    
                    stdio_printf("Fichier : %s", filename);
                }
                stdio_printf(", Cluster : %u, Taille : %u octets\n", file_cluster, size);
                
                // _fatfs_read_file(boot, file_cluster, size);
            }
        }
    }
}

void _fatfs_format_short_name(char *filename) {
    char clean_name[13] = {0};
    int j = 0;
    int i = 0;
    int has_extension = 0;

    // Supprimer les espaces inutiles dans le nom
    for (i=0; i < 8; i++) {
        if (filename[i] != ' ') {
            clean_name[j++] = filename[i];
        }
    }

    // Vérifier si une extension existe (non vide)
    for (int i = 9; i < 12; i++) {
        if (filename[i] != ' ') {
            has_extension = 1;
            break;
        }
    }

    // Ajouter l'extension si nécessaire
    if (has_extension) {
        for (i = 8; i < 12; i++) {
            if (filename[i] != ' ') {
                clean_name[j++] = filename[i];
            }
        }
    }

    clean_name[j] = '\0';  // Terminer la chaîne proprement

    for (i = 0; clean_name[i]; i++) {
        if (clean_name[i] >= 'A' && clean_name[i] <= 'Z') {
            clean_name[i] += 32;  // Convertir en minuscule
        }
    }

    strcpy(filename, clean_name);
}


uint32_t _fatfs_cluster_to_sector_from_data_start(fat_bs_t *boot, uint32_t cluster) {
    return boot->data_start + (cluster - 2) * boot->sectors_per_cluster;
}


// bool _fatfs_read_file(fat_bs_t *boot, uint32_t start_cluster, uint32_t file_size) {
//     block_t buffer;
//     uint32_t cluster = start_cluster;
//     uint32_t bytes_read = 0;

//     // stdio_printf("Lecture du fichier (Cluster %lu, Taille %lu octets)\n", start_cluster, file_size);

//     while (cluster < 0x0FFFFFF7) {  // Tant qu'on n'est pas à la fin du fichier
//         uint32_t sector = _fatfs_cluster_to_sector_from_data_start(boot, cluster);

//         for (uint32_t s = 0; s < boot->sectors_per_cluster; s++) {
//             if (!sd_read_block(sector + s, buffer)) {
//                 stdio_printf("[ERROR] Impossible de lire le fichier\n");
//                 return false;
//             }

//             for (int i = 0; i < 512; i++) {
//                 if (bytes_read >= file_size) {
//                     // stdio_printf("\nFIN DU FICHIER\n");
//                     return true;
//                 }

//                 stdio_printf("%c", buffer[i]);
//                 bytes_read++;
//             }
//         }

//         cluster = _fatfs_read_fat_entry(boot, cluster);
//     }
//     stdio_printf("\n");  // Pour un affichage correct

//     // stdio_printf("\nFIN DU FICHIER\n");
//     return true;
// }

#endif