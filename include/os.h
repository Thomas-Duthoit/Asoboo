#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "pico/stdlib.h"
#include "pico/printf.h"
#include "init_hardware.h"
#include <ctype.h>
#include "icons.h"




#define TARGET_FILE_EXT "boo"
#define GAME_COL_MAX_GAME_COUNT 20
#define MAX_LINE_LENGTH 64












typedef struct {
    char game_name[256];
    uint32_t cluster;
    uint32_t size;
} fs_game_loc_t;

typedef struct {
    fs_game_loc_t games[GAME_COL_MAX_GAME_COUNT];
    uint16_t game_count;
} fs_game_collection;


void        os_init                                      ();
void        _os_add_game_to_collection                   (fs_game_collection *c, uint32_t cluster, char *name, uint32_t size);
void        os_scan_games_in_root                        (fs_game_collection *c, fat_bs_t *boot);
void        os_main_menu                                 ();
void        os_run_game                                  (fs_game_loc_t game);
void        _os_run_script_from_cluster                  (uint32_t cluster, uint32_t file_size);
char        _os_process_script_command                   (const char *line, uint32_t cluster, int* cmd_exit);
void        _os_set_variable                            (const char *name, const char *value_expr);
int         _os_get_variable_value                       (const char *name);
int         _os_parse_number                             (const char *str);
int         _os_evaluate_expression                      (const char *expr);
void        _os_replace_vars_in_text                     (const char *input, char *output, int output_size);



fs_game_collection GAMES = {0};

















#define _CMD_CODE_OK 1
#define _CMD_CODE_GOTO 2

#define _EXEC_VARIABLES_COUNT 50

typedef struct {
    char name[20];
    int value;
} prog_var_t;

typedef struct {
    prog_var_t vars[_EXEC_VARIABLES_COUNT];
    int v_count;
} prog_vars_t;



prog_vars_t _VARS = {0};


















void os_init() {
    init_hardware();
    os_scan_games_in_root(&GAMES, &BOOT);
    stdio_printf("---------------------\n");
    for (int i=0; i<GAMES.game_count; i++) {
        stdio_printf("JEU: %s (%d)\n", GAMES.games[i].game_name, GAMES.games[i].cluster);
    }
    stdio_printf("---------------------\n");
}


void os_scan_games_in_root(fs_game_collection *c, fat_bs_t *boot) {
    block_t buffer;
    uint32_t cluster = boot->root_cluster;

    uint32_t sector = _fatfs_cluster_to_sector_from_data_start(boot, cluster);

    stdio_printf("Scan des jeux à la racine (Cluster %lu)\n", cluster);

    char long_filename[256] = {0};  // Stocke le nom long
    char filename[12] = {0};        // Stocke le nom court
    char *extension;

    c->game_count = 0;

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
                    stdio_printf("%s/ (répértoire ignoré)\n", long_filename);
                    memset(long_filename, 0, sizeof(long_filename));  // Réinitialiser après affichage
                } else {
                    memcpy(filename, &buffer[j], 8);                   
                    _fatfs_format_short_name(filename);  // Nettoyer les espaces inutiles
                    stdio_printf("%s/ (répértoire ignoré)\n", filename);
                }

            } else {  // FICHIER
                if (long_filename[0]) {
                    stdio_printf("Fichier : %s", long_filename);

                    extension = strtok(long_filename, ".");
                    extension = strtok(NULL, ".");

                    if (strcmp(extension, TARGET_FILE_EXT)) continue;

                    _os_add_game_to_collection(c, file_cluster, long_filename, size);

                    // stdio_printf("EXTENSION : \"%s\"\n", extension);

                    memset(long_filename, 0, sizeof(long_filename));  // Réinitialiser après affichage
                } else {
                    memcpy(filename, &buffer[j], 8);
                    filename[8] = '.';
                    memcpy(&filename[9], &buffer[j + 8], 3);
                    
                    _fatfs_format_short_name(filename);  // Nettoye r les espaces inutiles


                    
                    stdio_printf("Fichier : %s", filename);

                    extension = strtok(filename, ".");
                    extension = strtok(NULL, ".");

                    if (strcmp(extension, TARGET_FILE_EXT)) continue;


                    _os_add_game_to_collection(c, file_cluster, filename, size);
                    // stdio_printf("EXTENSION : \"%s\"\n", extension);
                    
                }
                stdio_printf(", Cluster : %u, Taille : %u octets\n", file_cluster, size);
                
            }
        }
    }
}


void _os_add_game_to_collection(fs_game_collection *c, uint32_t cluster, char *name, uint32_t size) {
    if (c->game_count == GAME_COL_MAX_GAME_COUNT) return;

    c->games[c->game_count].cluster = cluster;
    c->games[c->game_count].size = size;
    strcpy(c->games[c->game_count].game_name, name);

    for (int i=0; i<strlen(c->games[c->game_count].game_name); i++) {
        if (c->games[c->game_count].game_name[i] >= 'a' && c->games[c->game_count].game_name[i] <= 'z')
            c->games[c->game_count].game_name[i] += 'A' - 'a';
    }

    c->game_count++;
}





































void os_main_menu() {

    char buf[22];  // MAX 20 char on one line
    int game_idx=0;

    _render_boot_img();

    while (!get_btn_state(PIN_BTN_A))
    {
        if (get_btn_state(PIN_BTN_DOWN)) game_idx++;
        else if (get_btn_state(PIN_BTN_UP)) game_idx--;
        if (game_idx==GAMES.game_count) game_idx = 0;
        if (game_idx <0) game_idx = GAMES.game_count-1;

        g_fill_rect(2, 2, SCREEN_WIDTH-3, SCREEN_HEIGHT-3, BLACK);
        for (int i=0; i<GAMES.game_count; i++) {
            if (i==game_idx) {
                snprintf(buf, 300, "> %s", GAMES.games[i].game_name);
            } else {
                snprintf(buf, 300, "%s", GAMES.games[i].game_name);
            }
            g_draw_string(5+8, i*FONT_HEIGHT+5, buf, WHITE, BLACK);
            render_icon(5, i*FONT_HEIGHT, ICON_DATA);
        }

        sleep_ms(300);  // Eviter les clignotements

        while (!get_btn_state(PIN_BTN_DOWN) && !get_btn_state(PIN_BTN_UP) && !get_btn_state(PIN_BTN_A)) {} // attente
    }

    os_run_game(GAMES.games[game_idx]);  

}









































void os_run_game(fs_game_loc_t game) {
    stdio_printf("Chargement du jeu %s...\n", game.game_name);

    g_fill_rect(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
    _os_run_script_from_cluster(game.cluster, game.size);
}


void _os_run_script_from_cluster(uint32_t cluster, uint32_t file_size) {
    fat_bs_t *boot = &BOOT;
    block_t buffer;
    uint32_t bytes_read = 0;
    
    uint32_t sector;
    uint32_t actual_cluster = cluster;

    char line[MAX_LINE_LENGTH];
    int line_pos = 0;

    char cmd_code = _CMD_CODE_OK;
    int cmd_exit = 0;

    bool restart_goto = false;  // Indique si on doit recommencer le script

    int tmp;

    stdio_printf("Début de l'exécution du script (Cluster: %lu, Taille: %lu octets)\n", actual_cluster, file_size);

    while (actual_cluster < 0x0FFFFFF7) {
        sector = _fatfs_cluster_to_sector_from_data_start(boot, actual_cluster);

        for (uint32_t s = 0; s < boot->sectors_per_cluster; s++) {
            if (!sd_read_block(sector + s, buffer)) {
                stdio_printf("[ERROR] Impossible de lire le fichier\n");
                return;
            }

            for (int i = 0; i < 512 && bytes_read < file_size; i++) {

                char c = buffer[i];
                bytes_read++;

                if (c == '\n' || line_pos >= MAX_LINE_LENGTH - 1 || c == '\0') {
                    line[line_pos] = '\0';

                    stdio_printf("[DEBUG] Ligne lue: %s\n", line);

                    if (restart_goto) {
                        // On cherche le LABEL correspondant au GOTO
                        if (sscanf(line, "LABEL %d", &tmp) == 1 && tmp == cmd_exit) {
                            stdio_printf("[DEBUG] GOTO atteint le LABEL %d !\n", cmd_exit);
                            restart_goto = false;  // On reprend l'exécution normale
                            cmd_code = _CMD_CODE_OK;
                        }
                    } 
                    else {
                        stdio_printf(" (EXECUTION NORMALE)\n");
                        cmd_code = _os_process_script_command(line, actual_cluster, &cmd_exit);
                        if (cmd_code == _CMD_CODE_GOTO) {
                            restart_goto = true;  // On active la recherche du LABEL
                            actual_cluster = cluster;  // On recommence le fichier
                            bytes_read = 0;
                            break;  // Quitter la boucle et redémarrer
                        }
                    }

                    line_pos = 0;
                } else {
                    line[line_pos++] = c;
                }
            }
            if (restart_goto) break;
        }

        if (restart_goto) {
            actual_cluster = cluster;  // On recommence au début du fichier
            bytes_read = 0;
        } else {
            actual_cluster = _fatfs_read_fat_entry(boot, actual_cluster);
        }
    }
}



char _os_process_script_command(const char *line, uint32_t cluster, int* cmd_exit) {
    char param1[32], param2[32], param3[32], param4[32], param5[32];
    char text[64], parsed_text[64];

    if (sscanf(line, "RECT %s %s %s %s %s", param1, param2, param3, param4, param5) == 5) {
        g_fill_rect(
            _os_evaluate_expression(param1),
            _os_evaluate_expression(param2),
            _os_evaluate_expression(param3),
            _os_evaluate_expression(param4),
            _os_evaluate_expression(param5)
        );
    } 
    else if (sscanf(line, "TEXT %s %s \"%[^\"]\"", param1, param2, text) == 3) {
        _os_replace_vars_in_text(text, parsed_text, sizeof(parsed_text));
        g_draw_string(
            _os_evaluate_expression(param1),
            _os_evaluate_expression(param2),
            parsed_text, 0xFFFF, 0x0000
        );
    } 
    else if (sscanf(line, "WAIT %s", param1) == 1) {
        sleep_ms(_os_evaluate_expression(param1));
    }
    else if (sscanf(line, "GOTO %s", param1) == 1) {
        *cmd_exit = _os_evaluate_expression(param1);
        return _CMD_CODE_GOTO;
    }
    else if (sscanf(line, "LABEL %s", param1) == 1) {
        // Label, ne rien faire
    } 
    else if (strncmp(line, "SET", 3) == 0) {
        char var[32], expr[64];
        if (sscanf(line, "SET %31s %63[^\n]", var, expr) == 2) {
            _os_set_variable(var, expr);
        } else {
            stdio_printf("[ERROR] Mauvais format pour SET : %s\n", line);
        }
    }
    else {
        stdio_printf("Commande inconnue: %s\n", line);
    }

    return _CMD_CODE_OK;
}
































void _os_set_variable(const char *name, const char *value_expr) {
    char full_name[32];

    if (name[0] != '$') {  // Ajouter `$` si absent
        snprintf(full_name, sizeof(full_name), "$%s", name);
        name = full_name;
    }

    int result = _os_evaluate_expression(value_expr);  // Évaluer l'expression

    for (int i = 0; i < _VARS.v_count; i++) {
        if (strcmp(_VARS.vars[i].name, name) == 0) {
            _VARS.vars[i].value = result;
            stdio_printf("[DEBUG] Mise à jour de la variable : %s = %d\n", name, result);
            return;
        }
    }

    if (_VARS.v_count < _EXEC_VARIABLES_COUNT) {
        strcpy(_VARS.vars[_VARS.v_count].name, name);
        _VARS.vars[_VARS.v_count].value = result;
        stdio_printf("[DEBUG] Nouvelle variable créée : %s = %d\n", name, result);
        _VARS.v_count++;
    } else {
        stdio_printf("[ERROR] Trop de variables stockées !\n");
    }
}

int _os_get_variable_value(const char *name) {
    char full_name[32];
    if (name[0] != '$') {  // Si `$` absent, l'ajouter
        snprintf(full_name, sizeof(full_name), "$%s", name);
        name = full_name;
    }

    for (int i = 0; i < _VARS.v_count; i++) {
        if (strcmp(_VARS.vars[i].name, name) == 0) {
            return _VARS.vars[i].value;
        }
    }
    stdio_printf("[ERROR] Variable non trouvée : %s, valeur par défaut = 0\n", name);
    return 0;
}


int _os_parse_number(const char *str) {
    int result = 0;
    
    // Vérifie si c'est un hexadécimal (commence par '0x' ou juste 'x')
    if ((str[0] == '0' && str[1] == 'x') || (str[0] == 'x')) {
        str += (str[0] == '0') ? 2 : 1; // Ignore "0x" ou "x"
        while (*str) {
            result *= 16;
            if (*str >= '0' && *str <= '9') result += *str - '0';
            else if (*str >= 'A' && *str <= 'F') result += *str - 'A' + 10;
            else if (*str >= 'a' && *str <= 'f') result += *str - 'a' + 10;
            else break; // Stop si caractère invalide
            str++;
        }
    } 
    else { 
        // Sinon, conversion normale en décimal (comme `_os_parse_number`)
        int sign = 1;
        if (*str == '-') { sign = -1; str++; }
        while (*str >= '0' && *str <= '9') {
            result = result * 10 + (*str - '0');
            str++;
        }
        result *= sign;
    }
    
    return result;
}

int _os_evaluate_expression(const char *expr) {
    char left[32], right[32], op;
    int lval, rval;

    if (sscanf(expr, "%31s %c %31s", left, &op, right) == 3) {
        // Remplacer les variables si nécessaire
        lval = (left[0] == '$') ? _os_get_variable_value(left) : _os_parse_number(left);
        rval = (right[0] == '$') ? _os_get_variable_value(right) : _os_parse_number(right);

        switch (op) {
            case '+': return lval + rval;
            case '-': return lval - rval;
            case '*': return lval * rval;
            case '/': return (rval != 0) ? lval / rval : 0;  // Éviter la division par zéro
            default:
                stdio_printf("[ERROR] Opérateur inconnu : %c\n", op);
                return 0;
        }
    }

    // Si c'est juste un nombre ou une variable
    return (expr[0] == '$') ? _os_get_variable_value(expr) : _os_parse_number(expr);
}

void _os_replace_vars_in_text(const char *input, char *output, int output_size) {
    int i = 0, j = 0;
    char var_name[32];
    int var_value;
    
    stdio_printf("[DEBUG] Remplacement des variables dans : \"%s\"\n", input);

    while (input[i] != '\0' && j < output_size - 1) {
        if (input[i] == '$') {  // Détection d'une variable
            int k = 0;
            i++;  // Sauter le `$`
            while (isalnum(input[i]) && k < sizeof(var_name) - 1) {  // Lire le nom de la variable
                var_name[k++] = input[i++];
            }
            var_name[k] = '\0';

            // Ajouter `$` pour la recherche
            char full_var_name[32];
            snprintf(full_var_name, sizeof(full_var_name), "$%s", var_name);
            
            var_value = _os_get_variable_value(full_var_name);
            stdio_printf("[DEBUG] Variable trouvée : %s = %d\n", full_var_name, var_value);

            j += snprintf(output + j, output_size - j, "%d", var_value);
        } else {
            output[j++] = input[i++];
        }
    }
    output[j] = '\0';

    stdio_printf("[DEBUG] Résultat après remplacement : \"%s\"\n", output);
}


#endif