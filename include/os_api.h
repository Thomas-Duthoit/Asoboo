#ifndef OS_API_H
#define OS_API_H


/*
Include this file when creating a game for Asoboo to access hardware level function
*/


// GAME TEMPLATE
/*
#include "os_api.h"  // include this file

__attribute__((used)) __attribute__((section(".text.game_main"))) // USED TO BE KEPT BY THE COMPILER
void game_main(os_api_t* api) {  // game entry point
    // do something ...
    // to use an api function :
    // api->function(args);
    return;
}
*/



#include <stdint.h>
#include <stddef.h>

typedef struct 
{
    int (*print)(const char *, ...);  // printf through USB serial
    void (*bl)(char);                 // control display backlight (0=off, 1=on)
} os_api_t;



#endif // OS_API_H
