#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include "printf_log.h"

// Given the path to a serial device, open the device and configure it.
// Return the file descriptor associated with the device.
int OpenSerialPort(const char *bsdPath, int should_block, speed_t speed)
{
	int						fileDescriptor = -1;
	struct termios			options;

	// Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
	// The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
	// See open(2) ("man 2 open") for details.

	fileDescriptor = open(bsdPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fileDescriptor == -1 || flock(fileDescriptor, LOCK_EX) == -1)
	{
		printfLog("Error opening serial port %s - %s(%d).\n",
				 bsdPath, strerror(errno), errno);
		goto error;
	}
	// Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block.
	// See fcntl(2) ("man 2 fcntl") for details.
	if (fcntl(fileDescriptor, F_SETFL, 0) == -1)
	{
		printfLog("Error clearing O_NONBLOCK %s - %s(%d).\n",
			bsdPath, strerror(errno), errno);
		goto error;
	}

	if (ioctl(fileDescriptor, TIOCEXCL, (char *) 0) < 0) {
		printfLog("Error setting TIOCEXCL %s - %s(%d).\n",
			bsdPath, strerror(errno), errno);
		goto error;
	}
	memset(&options, 0, sizeof(options));
	// The baud rate, word length, and handshake options can be set as follows:
	options.c_iflag = 0;
	options.c_oflag = 0;

	options.c_cflag = CS8|CREAD|CLOCAL|CSTOPB;			 // 8n2, see termios.h for more information
	options.c_lflag = 0;
	options.c_cc[VMIN]  = should_block? 1: 0;
	options.c_cc[VTIME] = 10;
	cfsetospeed(&options, speed);			// Set baud
	cfsetispeed(&options, speed);

	tcflush(fileDescriptor, TCIFLUSH);
	//cfmakeraw(&options);

	// Cause the new options to take effect immediately.
	if (tcsetattr(fileDescriptor, TCSANOW, &options) == -1)
	{
		printfLog("Error setting tty attributes %s - %s(%d).\n",
			bsdPath, strerror(errno), errno);
		goto error;
	}

		// Success
	return fileDescriptor;

	// Failure path
error:
	if (fileDescriptor != -1)
	{
		close(fileDescriptor);
	}

	return -1;
}
