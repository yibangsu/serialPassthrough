#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "main.h"
#include "printf_log.h"
#include "serial_interface.h"

void printfHelp(const char *prog)
{
    printfLog("Use %s [OPTION] <SERIAL A> <SERIAL B>\n", prog);
	printfLog("Examples:\n");
	printfLog("\n");

	printfLog("  -h, --help                Show this help message\n");
	printfLog("  -b, --block               Set block\n");
    printfLog("  -s, --speed <baud rate>   Set baud rate\n");
    printfLog("                            Example baud rate:\n");
    printfLog("                              9600\n");
    printfLog("                              19200\n");
    printfLog("                              115200\n");
    printfLog("                              921600\n");
    printfLog("                              1000000\n");
    printfLog("                              1152000\n");
    printfLog("                              1500000\n");

	printfLog("\n");
	printfLog("<SERIAL A> <SERIAL B>:\n");
    printfLog("  serials to pass through\n");
}

#ifdef __PRINT_SETTING__
static char* convertSpeedStr(speed_t speed)
{
    char* speed_str = "Baud rate unsupported";
    switch (speed)
    {
    case B9600:
        speed_str = "9600";
        break;

    case B19200:
        speed_str = "19200";
        break;

    case B115200:
        speed_str = "115200";
        break;

    case B921600:
        speed_str = "921600";
        break;

    case B1000000:
        speed_str = "1000000";
        break;

    case B1152000:
        speed_str = "1152000";
        break;

    case B1500000:
        speed_str = "1500000";
        break;
    
    default:
        break;
    }
    return speed_str;
}

static void printfSettings(sUartPassSetting_t setting)
{
    printfLog("Setting:\n");
    printfLog("  serial_A :%s\n", setting.serial_A);
    printfLog("  serial_B :%s\n", setting.serial_B);
    printfLog("  block    :%d\n", !!setting.block);
    printfLog("  speed    :%s[0x%08x]\n", convertSpeedStr(setting.speed), setting.speed);
}
#endif

static speed_t convertSpeed(const char* strSpeed)
{
    speed_t speed = B0;
    if (strSpeed)
    {
        if (strncmp(strSpeed, "9600", strlen("9600")) == 0)
            return B9600;
        if (strncmp(strSpeed, "19200", strlen("19200")) == 0)
            return B19200;
        if (strncmp(strSpeed, "115200", strlen("115200")) == 0)
            return B115200;
        if (strncmp(strSpeed, "921600", strlen("921600")) == 0)
            return B921600;
        if (strncmp(strSpeed, "1000000", strlen("1000000")) == 0)
            return B1000000;
        if (strncmp(strSpeed, "1152000", strlen("1152000")) == 0)
            return B1152000;
        if (strncmp(strSpeed, "1500000", strlen("1500000")) == 0)
            return B1500000;
    }
    return speed;
}

static int checkSerials(sUartPassSetting_t setting)
{
    if (B0 == setting.speed)
    {
        printfLog("Speed is B0, unsupported\n\n");
        return 0;
    }
        
    if (!setting.serial_A)
    {
        printfLog("Serial A is NULL\n\n");
        return 0;
    }

    if (!setting.serial_B)
    {
        printfLog("Serial B is NULL\n\n");
        return 0;
    }

    if (strcmp(setting.serial_A, setting.serial_B) == 0)
    {
        printfLog("Serial A and Serial B are the same [%s]\n\n", setting.serial_A);
        return 0;
    }
    
    return 1;
}

static int passthrough(int fd_in, int fd_out)
{
    #define BUFF_SIZE       1024
    char buff[BUFF_SIZE] = { 0 };
    int  size = 0;

    while (1)
    {
        size = read(fd_in, buff, sizeof(buff));
        if (size < 0)
        {
            break;
        }
        if (size > 0)
        {
            size = write(fd_out, buff, size);
            if (size < 0)
            {
                break;
            }
        }
    }

    return ERROR_PASS_THROUGH;
}

int main(int argc, char* argv[])
{
    sUartPassSetting_t  uart_setting = { 0 };
    char                show_help = 0;
    int                 fd_A = -1;
    int                 fd_B = -1;

    int c;
    while (1)
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"speed", required_argument,    0, 's'},
            {"block", no_argument,          0, 'b'},
            {"help",  no_argument,          0, 'h'},
            {0,       0,                    0,  0 },
        };

        c = getopt_long(argc, argv, "s:bh", long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
        case 's':
            uart_setting.speed = convertSpeed(optarg);
            break;
        
        case 'b':
            uart_setting.block = 1;
            break;

        case 'h':
            show_help = 1;
            break;

        default:
            break;
        }
    }
    
    if (show_help)
    {
        printfHelp(argv[0]);
        return CODE_SUCCESS;
    }

    if (optind != argc - 2)
    {
        printfLog("Missing arguments\n\n");
        printfHelp(argv[0]);
        return ERROR_ARGUMENT;
    }

    uart_setting.serial_A = argv[argc - 2];
    uart_setting.serial_B = argv[argc - 1];

    if (!checkSerials(uart_setting))
    {
        printfHelp(argv[0]);
        return ERROR_SETTING;
    }

#ifdef __PRINT_SETTING__
    printfSettings(uart_setting);
#endif

    fd_A = OpenSerialPort(uart_setting.serial_A, uart_setting.block, uart_setting.speed);
    if (0 > fd_A)
    {
        printfLog("Open %d error\n\n");
        return ERROR_SERAIL_OPEN;
    }

    fd_B = OpenSerialPort(uart_setting.serial_B, uart_setting.block, uart_setting.speed);
    if (0 > fd_B)
    {
        printfLog("Open %d error\n\n");
        return ERROR_SERAIL_OPEN;
    }

    pid_t childPid1 = fork();
    if (childPid1 == 0)
    {
        eErrorCode_t code = passthrough(fd_A, fd_B);
        exit(code);
    }
    else
    {
        printfLog("fork %d: %s --> %s\n", childPid1, uart_setting.serial_A, uart_setting.serial_B);
    }

    pid_t childPid2 = fork();
    if (childPid2 == 0)
    {
        eErrorCode_t code = passthrough(fd_B, fd_A);
        exit(code);
    }
    else
    {
        printfLog("fork %d: %s --> %s\n", childPid2, uart_setting.serial_B, uart_setting.serial_A);
    }

    int wstatus = 0;
    waitpid(-1, &wstatus, 0);

    close(fd_A);
    close(fd_B);

    return CODE_SUCCESS;
}