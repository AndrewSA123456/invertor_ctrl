#include "backgroundTasks.h"
#include "IntelHexParser.h"
#include "aes.h"
#include "SocketCAN.h"

#define SOCKET_NAME "can0"

gint encryptFirmware(ExtractedFirmwareStruct *extractedData);
int vectorsAddrTransmit(ExtractedFirmwareStruct *extractedData);
int programAddrTransmit(ExtractedFirmwareStruct *extractedData);
int rebootCMDTransmit();
int programTransmit(ExtractedFirmwareStruct *extractedData);
int programSegmentTransmitInit(ExtractedFirmwareStruct *extractedData);
int programSegmentTransmit(uint8_t *buff, uint16_t typeMSG, uint32_t buff_size);
int waitingSDOResponse(uint32_t nodeID);
CANOpenMSGTask *create_MSG(
	uint32_t nodeID,
	uint16_t typeMSG,
	uint16_t index,
	uint8_t subindex,
	uint32_t len,
	uint8_t *data);
gpointer free_MSG(CANOpenMSGTask *MSG);
CANOpenMSGTask *CANOpenReceive(struct can_frame *CANframeReceived);
void CANOpenTransmit(int sockDesc, CANOpenMSGTask *TransmitMSG);
/////////////////////////////////////////////////////////////////////
// Поток: выполнение фоновых задач
gpointer backgroundTasksThread(gpointer data)
{
	ExtractedFirmwareStruct extractedData = {0, 0, NULL};
	gboolean treadRunning = TRUE;
	do
	{
		TaskStruct *task = g_async_queue_pop(Background.Queue);
		switch (task->CMD)
		{
		case TASK_PARSE_INTEL_HEX:
		{
			printf("TASK_PARSE_INTEL_HEX\n");
			if (IntelHexParser(firmware_file_name, &extractedData))
			{
				perror("IntelHexParser failed\n");
				treadRunning = FALSE;
			}
			// for(int i = 0;i<extractedData.program_data_size;i++)
			// {
			// 	if(i%16 == 0)
			// 		printf("\n");
			// 	printf("%X	", extractedData.program_data[i]);
			// }
		}
		break;
		case TASK_ENCRYPT_FIRMWARE:
		{
			printf("TASK_ENCRYPT_FIRMWARE\n");
			if (encryptFirmware(&extractedData))
			{
				perror("encryptFirmware failed\n");
				treadRunning = FALSE;
			}
		}
		break;
		case TASK_TRANSMIT_FIRMWARE:
		{
			printf("TASK_TRANSMIT_FIRMWARE\n");
			// if (vectorsAddrTransmit(&extractedData) == EXIT_FAILURE)
			// {
			// 	printf("vectorsAddrTransmit failure\n");
			// 	user_free(extractedData.program_data);
			// 	break;
			// }
			// if (programAddrTransmit(&extractedData) == EXIT_FAILURE)
			// {
			// 	printf("programAddrTransmit failure\n");
			// 	user_free(extractedData.program_data);
			// 	break;
			// }
			// if (rebootCMDTransmit(&extractedData) == EXIT_FAILURE)
			// {
			// 	printf("rebootCMDTransmit failure\n");
			// 	user_free(extractedData.program_data);
			// 	break;
			// }
			if (programTransmit(&extractedData) == EXIT_FAILURE)
			{
				printf("programTransmit failure\n");
				user_free(extractedData.program_data);
				break;
			}
			user_free(extractedData.program_data);
		}
		break;
		case TASK_EXIT:
		{
			printf("TASK_EXIT\n");
			treadRunning = FALSE;
		}
		break;
		default:
		{
		}
		break;
		}
		free_task(task);
	} while (treadRunning);
	user_free(extractedData.program_data);
	return NULL;
}
/////////////////////////////////////////////////////////////////////
// Поток: связь по CANOpen
#define POLL_EVENT_TIMEOUT 1000
gpointer CANOpenCommThread(gpointer data)
{
	gboolean treadRunning = TRUE;
	struct can_frame CANframeReceived = {0};
	int sockDesc = openSocketCAN(SOCKET_NAME);
	if (sockDesc < 0)
	{
		perror("openSocketCAN failed\n");
		return NULL;
	}
	g_async_queue_push(CANOpenComm.Queue, create_task(COMM_TASK_WAIT_MESSAGE, 0, NULL));
	do
	{
		TaskStruct *task = g_async_queue_pop(CANOpenComm.Queue);
		switch (task->CMD)
		{
		case COMM_TASK_WAIT_MESSAGE:
		{
			if (socketCANReceiveNotBlocking(sockDesc, &CANframeReceived, POLL_EVENT_TIMEOUT) == SOCKET_CAN_NEW_MSG)
			{
				// Добавляем сообщение в очередь обработки
				CANOpenMSGTask *ReceiveMSG = CANOpenReceive(&CANframeReceived);
				if (ReceiveMSG != NULL)
				{
					g_async_queue_push(incomingQueue, ReceiveMSG);
				}
			}
			g_async_queue_push(CANOpenComm.Queue, create_task(COMM_TASK_MSG_TRANSMIT, 0, NULL));
		}
		break;
		case COMM_TASK_MSG_TRANSMIT:
		{
			CANOpenMSGTask *TransmitMSG = g_async_queue_try_pop(outgoingQueue);
			if (TransmitMSG != NULL)
			{
				CANOpenTransmit(sockDesc, TransmitMSG);
			}
			free_MSG(TransmitMSG);
			g_async_queue_push(CANOpenComm.Queue, create_task(COMM_TASK_WAIT_MESSAGE, 0, NULL));
		}
		break;
		case COMM_TASK_EXIT:
		{
			printf("COMM_TASK_EXIT\n");
			treadRunning = FALSE;
		}
		break;
		default:
		{
		}
		break;
		}
		free_task(task);
	} while (treadRunning);
	closeSocketCAN(sockDesc);
	return NULL;
}
/////////////////////////////////////////////////////////////////////
// Функция: Шифрование прошивки
uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
gint encryptFirmware(ExtractedFirmwareStruct *extractedData)
{
	printf("encryptFirmware\n");
	if (extractedData == NULL)
	{
		perror("extractedData is NULL\n");
		return EXIT_FAILURE;
	}
	// Выравнивание размера программы до 16 байт
	uint32_t size = extractedData->program_data_size;
	if (size % 16 != 0)
	{
		size += 16 - size % 16;
	}
	extractedData->program_data = (uint8_t *)realloc(extractedData->program_data, size);
	// Инициализация вновь выделенной памяти
	for (int i = extractedData->program_data_size; i < size; i++)
	{
		extractedData->program_data[i] = 0x0;
	}
	extractedData->program_data_size = size;
	// Шифрование программы
	uint8_t encryptBuff[16] = {0};
	struct AES_ctx ctx;
	AES_init_ctx(&ctx, key);
	for (int i = 0; i < extractedData->program_data_size; i += 16)
	{
		memcpy(encryptBuff, &extractedData->program_data[i], 16);
		// printf("%X.	",i);
		// for(int j = 0; j<16; j++)
		// {
		// 	if(j==8)
		// 		printf("		");
		// 	printf("%X.",encryptBuff[j]);
		// }
		// printf("\n");
		AES_ECB_encrypt(&ctx, encryptBuff);
		memcpy(&extractedData->program_data[i], encryptBuff, 16);
	}
	return EXIT_SUCCESS;
}
/////////////////////////////////////////////////////////////////////
// Функция:
CANOpenMSGTask *create_MSG(
	uint32_t nodeID,
	uint16_t typeMSG,
	uint16_t index,
	uint8_t subindex,
	uint32_t len,
	uint8_t *data)
{
	CANOpenMSGTask *MSG = g_new(CANOpenMSGTask, 1);
	MSG->nodeID = nodeID;
	MSG->typeMSG = typeMSG;
	MSG->index = index;
	MSG->subindex = subindex;
	MSG->len = len;
	MSG->data = (uint8_t *)g_memdup2(data, len);
	return MSG;
}
/////////////////////////////////////////////////////////////////////
// Функция:
gpointer free_MSG(CANOpenMSGTask *MSG)
{
	if (MSG)
	{
		user_free(MSG->data);
		user_free(MSG);
	}
}
/////////////////////////////////////////////////////////////////////
// Функция:
// Запрос:	600+NodeID  8байт   22  51  1F  02  Адрес начала таблицы векторов прерываний
// Ответ:	580+NodeID  4байта  60  51  1F  02
// cansend vcan0 582#60511F02
int vectorsAddrTransmit(ExtractedFirmwareStruct *extractedData)
{
	uint32_t nodeID = SDO_TX;
	uint16_t typeMSG = SDO_EXPEDITED_TRANSFER;
	uint16_t index = 0x1F51;
	uint8_t subindex = 0x02;
	uint32_t len = 4;
	uint8_t vectors_address[4] = {0};
	for (int i = 0; i < 4; i++)
	{
		vectors_address[i] = (uint8_t)((extractedData->vectors_address >> (i * 8)) & 0xFF);
	}
	g_async_queue_push(outgoingQueue, create_MSG(nodeID,
												 typeMSG,
												 index,
												 subindex,
												 len,
												 vectors_address));
	return waitingSDOResponse(SDO_RX);
}
/////////////////////////////////////////////////////////////////////
// Функция:
// Запрос:	600+NodeID  8байт   22  51  1F  03  Адрес начала кода программы
// Ответ:	580+NodeID  4байта  60  51  1F  03
// cansend vcan0 582#60511F03
int programAddrTransmit(ExtractedFirmwareStruct *extractedData)
{
	uint32_t nodeID = SDO_TX;
	uint16_t typeMSG = SDO_EXPEDITED_TRANSFER;
	uint16_t index = 0x1F51;
	uint8_t subindex = 0x03;
	uint32_t len = 4;
	uint8_t programm_address[4] = {0};
	for (int i = 0; i < 4; i++)
	{
		programm_address[i] = (uint8_t)((extractedData->programm_address >> (i * 8)) & 0xFF);
	}
	g_async_queue_push(outgoingQueue, create_MSG(nodeID,
												 typeMSG,
												 index,
												 subindex,
												 len,
												 programm_address));
	return waitingSDOResponse(SDO_RX);
}
/////////////////////////////////////////////////////////////////////
// Функция:
// Запрос:	600+NodeID  8байт   22  51  1F  01  Команда на перезагрузку
// Ответ:	580+NodeID  4байта  60  51  1F  01  // Отвечаем после перезагрузки
// cansend vcan0 582#60511F01
int rebootCMDTransmit()
{
	uint32_t nodeID = SDO_TX;
	uint16_t typeMSG = SDO_EXPEDITED_TRANSFER;
	uint16_t index = 0x1F51;
	uint8_t subindex = 0x01;
	uint32_t len = 1;
	uint8_t rebootCMD = (uint8_t)REBOOT_CMD;
	g_async_queue_push(outgoingQueue, create_MSG(nodeID,
												 typeMSG,
												 index,
												 subindex,
												 len,
												 &rebootCMD));
	return waitingSDOResponse(SDO_RX);
}
/////////////////////////////////////////////////////////////////////
// Функция:
// Запрос:	600+NodeID  8байт   00  7байт прошивки
// Ответ:	580+NodeID  4байта  20
// cansend vcan0 582#20
// Запрос:	600+NodeID  8байт   10  7байт прошивки
// Ответ:	580+NodeID  1байт  30
// cansend vcan0 582#30
// Запрос:	600+NodeID  8байт   00  7байт прошивки
// Ответ:	580+NodeID  1байт  20
// cansend vcan0 582#20
// ...
// Запрос:	600+NodeID  8байт   11  7байт прошивки
// Ответ:	580+NodeID  1байт  30
// cansend vcan0 582#30
int programTransmit(ExtractedFirmwareStruct *extractedData)
{
	if (programSegmentTransmitInit(extractedData) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	uint8_t buff[7] = {0};
	uint32_t remaining_data_size = 0;
	uint32_t typeMSG = 0;
	for (int i = 0; i < extractedData->program_data_size; i += 7)
	{
		remaining_data_size = extractedData->program_data_size - i;
		uint32_t segment_size = (remaining_data_size >= 7) ? 7 : remaining_data_size;
		memcpy(buff, extractedData->program_data + i, segment_size);
		if (i + 7 > extractedData->program_data_size)
		{
			typeMSG = (typeMSG == SDO_NORMAL_TRANSFER) ? SDO_NORMAL_TRANSFER_LAST_TG : SDO_NORMAL_TRANSFER_LAST;
		}
		else
		{
			typeMSG = (typeMSG == SDO_NORMAL_TRANSFER) ? SDO_NORMAL_TRANSFER_TG : SDO_NORMAL_TRANSFER;
		}
		if (programSegmentTransmit(buff, typeMSG, segment_size) == EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
/////////////////////////////////////////////////////////////////////
// Функция:
// Запрос:	600+NodeID  8байт   21  50  1F  01  размер прошивки
// Ответ:	580+NodeID  4байта  60  50  1F  01
// cansend vcan0 582#60501F01
int programSegmentTransmitInit(ExtractedFirmwareStruct *extractedData)
{
	uint32_t nodeID = SDO_TX;
	uint16_t typeMSG = SDO_NORMAL_TRANSFER_INIT;
	uint16_t index = 0x1F50;
	uint8_t subindex = 0x01;
	uint32_t len = 4;
	uint8_t program_data_size[4] = {0};
	for (int i = 0; i < 4; i++)
	{
		program_data_size[i] = (uint8_t)((extractedData->program_data_size >> (i * 8)) & 0xFF);
	}
	g_async_queue_push(outgoingQueue, create_MSG(nodeID,
												 typeMSG,
												 index,
												 subindex,
												 len,
												 program_data_size));
	return waitingSDOResponse(SDO_RX);
}
/////////////////////////////////////////////////////////////////////
// Функция:
int programSegmentTransmit(uint8_t *buff, uint16_t typeMSG, uint32_t buff_size)
{
	uint32_t nodeID = SDO_TX;
	uint16_t index = 0;
	uint8_t subindex = 0;
	uint32_t len = buff_size;
	g_async_queue_push(outgoingQueue, create_MSG(nodeID,
												 typeMSG,
												 index,
												 subindex,
												 len,
												 buff));
	return waitingSDOResponse(SDO_RX);
}
/////////////////////////////////////////////////////////////////////
// Функция:
int waitingSDOResponse(uint32_t nodeID)
{
	uint32_t waiting_count = WAITING_COUNT_START;
	while (TRUE)
	{
		CANOpenMSGTask *MSG = g_async_queue_try_pop(incomingQueue);
		if (MSG == NULL)
		{
			g_usleep(1000);
			// waiting_count--;
			continue;
		}
		if (MSG->nodeID == nodeID)
		{
			if (MSG->typeMSG == SDO_OK)
			{
				free_MSG(MSG);
				return EXIT_SUCCESS;
			}
			else if (MSG->typeMSG == SDO_ABORT)
			{
				free_MSG(MSG);
				return EXIT_FAILURE;
			}
		}
		g_async_queue_push(incomingQueue, MSG);

		free_MSG(MSG);
		g_usleep(1000);
		// waiting_count--;
	}
	return EXIT_FAILURE;
}
/////////////////////////////////////////////////////////////////////
// Функция:
CANOpenMSGTask *CANOpenReceive(struct can_frame *CANframeReceived)
{
	uint32_t nodeID = CANframeReceived->can_id;
	uint32_t typeMSG;
	uint16_t index = 0;
	uint8_t subindex = 0;
	if (CANframeReceived->data[0] != 0x80)
	{
		typeMSG = SDO_OK;
	}
	else
	{
		typeMSG = SDO_ABORT;
	}
	uint32_t len = CANframeReceived->len;
	uint8_t *data = (uint8_t *)calloc(sizeof(uint8_t), len);
	memcpy(data, CANframeReceived->data, len);
	CANOpenMSGTask *MSG = create_MSG(nodeID, typeMSG, index, subindex, len, data);
	user_free(data);
	return MSG;
}
/////////////////////////////////////////////////////////////////////
// Функция:
void CANOpenTransmit(int sockDesc, CANOpenMSGTask *TransmitMSG)
{
	uint8_t data[8] = {0};
	uint8_t MSGLen = 8;

	switch (TransmitMSG->typeMSG)
	{
	case SDO_EXPEDITED_TRANSFER:
	{
		data[0] = SDO_EXPEDITED_TRANSFER_SRB_REQ;
		data[1] = (uint8_t)(TransmitMSG->index & 0xFF);
		data[2] = (uint8_t)((TransmitMSG->index >> 8) & 0xFF);
		data[3] = TransmitMSG->subindex;
		memcpy(data + 4, TransmitMSG->data, TransmitMSG->len);
	}
	break;
	case SDO_NORMAL_TRANSFER_INIT:
	{
		data[0] = SDO_NORMAL_TRANSFER_INIT_SRB_REQ;
		data[1] = (uint8_t)(TransmitMSG->index & 0xFF);
		data[2] = (uint8_t)((TransmitMSG->index >> 8) & 0xFF);
		data[3] = TransmitMSG->subindex;
		memcpy(data + 4, TransmitMSG->data, TransmitMSG->len);
	}
	break;
	case SDO_NORMAL_TRANSFER:
	{
		data[0] = SDO_NORMAL_TRANSFER_SRB_REQ;
		memcpy(data + 1, TransmitMSG->data, TransmitMSG->len);
	}
	break;
	case SDO_NORMAL_TRANSFER_TG:
	{
		data[0] = SDO_NORMAL_TRANSFER_SRB_REQ_TG;
		memcpy(data + 1, TransmitMSG->data, TransmitMSG->len);
	}
	break;
	case SDO_NORMAL_TRANSFER_LAST:
	{
		data[0] = SDO_NORMAL_TRANSFER_SRB_REQ_LAST;
		memcpy(data + 1, TransmitMSG->data, TransmitMSG->len);
	}
	break;
	case SDO_NORMAL_TRANSFER_LAST_TG:
	{
		data[0] = SDO_NORMAL_TRANSFER_SRB_REQ_LAST_TG;
		memcpy(data + 1, TransmitMSG->data, TransmitMSG->len);
	}
	break;
	default:
		break;
	}
	socketCANTransmit(sockDesc,
					  TransmitMSG->nodeID,
					  MSGLen,
					  data);
}
