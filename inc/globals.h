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

#include <linux/can.h>
#include <linux/can/raw.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glib-unix.h>

extern gchar *firmware_file_name;
extern gboolean bootloader_encryption_activate;
extern gboolean upload_inverter_firmware_flag;

enum
{
	TASK_WAIT,
	TAST_PARSE_INTEL_HEX,
	TASK_ENCRYPT_FIRMWARE,
	TASK_TRANSMIT_FIRMWARE,
	TASK_TRANSMIT_FIRMWARE_OK,
	TASK_EXIT
};
enum
{
	RECEIVE_TASK_WAIT,
	RECEIVE_TASK_WAIT_MESSAGE,
	RECEIVE_TASK_EXIT
};
enum
{
	TRANSMIT_TASK_WAIT,
	TRANSMIT_TASK_SEND_VECTORS_ADDR,
	TRANSMIT_TASK_SEND_PROGRAMM_ADDR,
	TRANSMIT_TASK_REBOOT,
	TRANSMIT_TASK_SEND_PROGRAMM_DATA,
	TRANSMIT_TASK_EXIT
};

typedef struct
{
	GAsyncQueue *Queue;
	GThread *Thread;
} AppThreadStruct;

typedef struct
{
	uint16_t CMD;
	uint16_t LEN;
	uint8_t *data;
} InterThreadTaskStruct;
InterThreadTaskStruct *create_task(uint16_t CMD,
								   uint16_t LEN,
								   uint8_t *data);
gpointer free_task(InterThreadTaskStruct *task);

void user_free(gpointer mem);
extern AppThreadStruct CANRx;
extern AppThreadStruct CANTx;
extern AppThreadStruct Background;
extern GAsyncQueue *CANReceiveMSGQueue;
// create_task(RECEIVE_TASK_EXIT, 0, NULL)
#endif //__GLOBALS_H