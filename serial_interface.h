#ifndef __LINUX_SERAIL_INTARFACE_H__
#define __LINUX_SERAIL_INTARFACE_H__

#include <termios.h>

int OpenSerialPort(const char *bsdPath, int should_block, speed_t speed);

#endif