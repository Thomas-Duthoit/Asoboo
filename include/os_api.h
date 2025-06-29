#ifndef OS_API_H
#define OS_API_H

#include <stdint.h>
#include <stddef.h>

typedef struct 
{
    int (*print)(const char *, ...);
    void (*bl)(char);
} os_api_t;



#endif // OS_API_H
