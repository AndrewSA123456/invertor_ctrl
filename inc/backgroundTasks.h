#ifndef __BACKGROUNDTASKS_H
#define __BACKGROUNDTASKS_H

#include "globals.h"

/////////////////////////////////////////////////////////////////////
// Поток:
gpointer backgroundTasksThread(gpointer);
/////////////////////////////////////////////////////////////////////
// Поток: связь по CANOpen
gpointer CANOpenCommThread(gpointer data);
#define NODE_ID 2
#define SDO_TX (0x600 + NODE_ID)
#define SDO_RX (0x580 + NODE_ID)

#define WAITING_COUNT_START 1000
#define REBOOT_CMD (0xABU)

#define SDO_EXPEDITED_TRANSFER_SRB_REQ (0x22U)
#define SDO_EXPEDITED_TRANSFER_SRB_RES (0x60U)
#define SDO_NORMAL_TRANSFER_INIT_SRB_REQ (0x21U)
#define SDO_NORMAL_TRANSFER_INIT_SRB_RES (0x60U)
#define SDO_NORMAL_TRANSFER_SRB_REQ (0x00U)
#define SDO_NORMAL_TRANSFER_SRB_RES (0x20U)
#define SDO_NORMAL_TRANSFER_SRB_REQ_TG (0x10U)
#define SDO_NORMAL_TRANSFER_SRB_RES_TG (0x30U)
#define SDO_NORMAL_TRANSFER_SRB_REQ_LAST (0x1U)
#define SDO_NORMAL_TRANSFER_SRB_RES_LAST (0x20U)
#define SDO_NORMAL_TRANSFER_SRB_REQ_LAST_TG (0x11U)
#define SDO_NORMAL_TRANSFER_SRB_RES_LAST_TG (0x30U)

enum
{
	SDO_EXPEDITED_TRANSFER=1,
	SDO_NORMAL_TRANSFER_INIT,
	SDO_NORMAL_TRANSFER,
	SDO_NORMAL_TRANSFER_TG,
	SDO_NORMAL_TRANSFER_LAST,
	SDO_NORMAL_TRANSFER_LAST_TG,
	SDO_OK,
	SDO_ABORT
};
enum
{
	COMM_TASK_WAIT_MESSAGE,
	COMM_TASK_MSG_RECEIVED,
	COMM_TASK_MSG_TRANSMIT,
	COMM_TASK_EXIT
};
typedef struct
{
	uint32_t nodeID;
	uint16_t typeMSG;
	uint16_t index;
	uint8_t subindex;
	uint32_t len;
	uint8_t *data;
} CANOpenMSGTask;

#endif //__BACKGROUNDTASKS_H