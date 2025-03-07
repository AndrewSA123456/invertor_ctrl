#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <poll.h>
#include <linux/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glib-unix.h>

enum
{
	TASK_WAIT,
	TASK_PARSE_INTEL_HEX,
	TASK_ENCRYPT_FIRMWARE,
	TASK_TRANSMIT_FIRMWARE,
	TASK_TRANSMIT_VECTORS_ADDR,
	TASK_TRANSMIT_PROGRAMM_ADDR,
	TASK_TRANSMIT_REBOOT,
	TASK_TRANSMIT_PROGRAMM_DATA,
	TASK_TRANSMIT_CRC,
	TASK_EXIT
};
extern gchar *firmware_file_name;

typedef struct
{
	GAsyncQueue *Queue;
	GThread *Thread;
} AppThreadStruct;

typedef struct
{
	uint32_t CMD;
	uint32_t LEN;
	uint8_t *data;
} TaskStruct;
TaskStruct *create_task(uint32_t CMD,
						uint32_t LEN,
						uint8_t *data);
gpointer free_task(TaskStruct *task);

void user_free(gpointer mem);
extern AppThreadStruct CANOpenComm;
extern AppThreadStruct Background;
extern GAsyncQueue *incomingQueue;
extern GAsyncQueue *outgoingQueue;
#endif //__GLOBALS_H