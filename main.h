#ifndef __MAIN_H__
#define __MAIN_H__

#include <termios.h>

typedef struct {
    const char* serial_A;
    const char* serial_B;
    speed_t     speed;
    char        block;
} sUartPassSetting_t;

typedef enum {
    CODE_SUCCESS = 0,
    ERROR_ARGUMENT,
    ERROR_SETTING,
    ERROR_SERAIL_OPEN,
    ERROR_PASS_THROUGH,
    CODE_MAX,
} eErrorCode_t;

#endif