#include "backgroundTasks.h"
#include "IntelHexParser.h"
#include "aes.h"
#include "SocketCAN.h"

gint encryptFirmware(ExtractedFirmwareStruct *extractedData);
/////////////////////////////////////////////////////////////////////
// Поток: выполнение фоновых задач
gpointer backgroundTasksThread(gpointer data)
{
	ExtractedFirmwareStruct extractedData = {0, 0, NULL};
	gboolean treadRunning = TRUE;
	do
	{
		InterThreadTaskStruct *task = g_async_queue_pop(Background.Queue);
		switch (task->CMD)
		{
		case TAST_PARSE_INTEL_HEX:
		{
			printf("TASK_PARSE_INTEL_HEX\n");
			if (IntelHexParser("stm32g474ret6.hex", &extractedData))
			{
				perror("IntelHexParser failed\n");
				treadRunning = FALSE;
			}
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
			uint8_t *buff = (uint8_t *)calloc(sizeof(extractedData.vectors_address), sizeof(uint8_t));
			memcpy(buff, &extractedData.vectors_address, sizeof(extractedData.vectors_address));
			g_async_queue_push(CANTx.Queue,
							   create_task(TRANSMIT_TASK_SEND_VECTORS_ADDR,
										   sizeof(extractedData.vectors_address),
										   buff));
			buff = (uint8_t *)realloc(buff, sizeof(extractedData.programm_address));
			memcpy(buff, &extractedData.programm_address, sizeof(extractedData.programm_address));
			g_async_queue_push(CANTx.Queue,
							   create_task(TRANSMIT_TASK_SEND_PROGRAMM_ADDR,
										   sizeof(extractedData.programm_address),
										   buff));
			user_free(buff);
			g_async_queue_push(CANTx.Queue,
							   create_task(TRANSMIT_TASK_SEND_PROGRAMM_DATA,
										   extractedData.program_data_size,
										   extractedData.program_data));
		}
		break;
		case TASK_EXIT:
		{
			printf("TASK_EXIT\n");
			treadRunning = FALSE;
		}
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
		AES_ECB_encrypt(&ctx, encryptBuff);
		memcpy(&extractedData->program_data[i], encryptBuff, 16);
	}
	return EXIT_SUCCESS;
}

#define NODE_ID 2
#define SDO_TX (0x600 + NODE_ID)
#define SDO_RX (0x580 + NODE_ID)
// typedef struct
// {
// 	uint8_t COBID
// };

/////////////////////////////////////////////////////////////////////
// Поток: передает сообщения по CAN
gpointer socketCANTransmitThread(gpointer data)
{
	int sockDesc = openSocketCAN("vcan0");
	if (sockDesc < 0)
	{
		perror("openSocketCAN failed\n");
		return NULL;
	}
	gboolean treadRunning = TRUE;
	do
	{
		InterThreadTaskStruct *task = g_async_queue_pop(CANTx.Queue);
		switch (task->CMD)
		{
		case TRANSMIT_TASK_WAIT:
		{
		}
		break;
		case TRANSMIT_TASK_SEND_VECTORS_ADDR:
		{
			printf("TRANSMIT_TASK_SEND_VECTORS_ADDR\n");
			// Запрос:	600+NodeID  8байт   22  51  1F  02  Адрес начала таблицы векторов прерываний
			// Ответ:	580+NodeID  4байта  60  51  1F  02
			// SDORequest()
		}
		break;
		case TRANSMIT_TASK_SEND_PROGRAMM_ADDR:
		{
			printf("TRANSMIT_TASK_SEND_PROGRAMM_ADDR\n");
			// Запрос:	600+NodeID  8байт   22  51  1F  03  Адрес начала кода программы
			// Ответ:	580+NodeID  4байта  60  51  1F  03
		}
		case TRANSMIT_TASK_REBOOT:
		{
			printf("TRANSMIT_TASK_REBOOT\n");
			// Запрос:	600+NodeID  8байт   22  51  1F  01  Команда на перезагрузку
			// Ответ:	580+NodeID  4байта  60  51  1F  01  // Отвечаем после перезагрузки
		}
		break;
		case TRANSMIT_TASK_SEND_PROGRAMM_DATA:
		{
			// Запрос:	600+NodeID  8байт   21  50  1F  01  размер прошивки
			// Ответ:	580+NodeID  4байта  60  50  1F  01
			// Запрос:	600+NodeID  8байт   00  7байт прошивки
			// Ответ:	580+NodeID  4байта  20
			// Запрос:	600+NodeID  8байт   10  7байт прошивки
			// Ответ:	580+NodeID  4байта  30
			// Запрос:	600+NodeID  8байт   00  7байт прошивки
			// Ответ:	580+NodeID  4байта  20
			// ...
			// Запрос:	600+NodeID  8байт   11  7байт прошивки
			// Ответ:	580+NodeID  4байта  30
			printf("TRANSMIT_TASK_SEND_PROGRAMM_DATA\n");
			uint8_t CANDataBuff[8] = {0};
			for (int i = 0; i < task->LEN; i += 7)
			{
				memcpy(&CANDataBuff[1], &task->data[i], 7);
				CANDataBuff[0] = 0x00;
				socketCANTransmit(sockDesc, 0x011, 8, CANDataBuff);
				// // Морозим поток до получения подтверждения
				// InterThreadTaskStruct *waitCANReceiveMSG = g_async_queue_pop(CANReceiveMSGQueue);
				// switch (waitCANReceiveMSG->CMD)
				// {
				// case TASK_TRANSMIT_FIRMWARE_OK:
				// {
				// 	printf("TASK_TRANSMIT_FIRMWARE_OK\n");
				// }
				// break;
				// default:
				// 	break;
				// }
				// free_task(waitCANReceiveMSG);
			}
			// Отправляем SDO по адресу 0x600 + 0xID
			// Активируем загрузчик записью в [1F51h,01h] = 0x01
			// Переходим в состояние TRANSMIT_TASK_WAIT_CONFIRM
			// Пишем очередной пакет данных программы в [1F50h,01h]
			// Переходим в состояние TRANSMIT_TASK_WAIT_CONFIRM
		}
		break;
		case TRANSMIT_TASK_EXIT:
		{
			printf("TRANSMIT_TASK_EXIT\n");
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
#define POLL_EVENT_TIMEOUT 1000
/////////////////////////////////////////////////////////////////////
// Поток: принимает сообщения по CAN
gpointer socketCANReceiveThread(gpointer data)
{
	gboolean treadRunning = TRUE;
	struct can_frame CANframe = {0};
	int sockDesc = openSocketCAN("vcan0");
	if (sockDesc < 0)
	{
		perror("openSocketCAN failed\n");
		return NULL;
	}

	// Настройка структуры pollfd
	struct pollfd sockfds[1];
	sockfds[0].fd = sockDesc;	// Сокет для мониторинга
	sockfds[0].events = POLLIN; // Интересующее событие: данные для чтения
	int RetPoll = 0;
	uint16_t CMD = RECEIVE_TASK_WAIT_MESSAGE;
	do
	{
		InterThreadTaskStruct *task = g_async_queue_try_pop(CANRx.Queue);
		if (task != NULL)
		{
			CMD = task->CMD;
		}
		switch (CMD)
		{
		case RECEIVE_TASK_WAIT:
		{
		}
		break;
		case RECEIVE_TASK_WAIT_MESSAGE:
		{
			RetPoll = poll(sockfds, 1, POLL_EVENT_TIMEOUT);
			if (RetPoll < 0)
			{
				perror("poll");
			}
			else if (RetPoll == 0)
			{
			}
			else
			{
				if (sockfds[0].revents & POLLIN)
				{
					sockfds[0].revents = 0;
					if (socketCANReceive(sockDesc, &CANframe) == SOCKET_CAN_OK)
					{
						for (int i = 0; i < CANframe.len; i++)
						{
							printf("%02X ", CANframe.data[i]);
						}
						printf("\n");
						// if (CANframe.can_id == 0x011)
						// {
						//     InterThreadTaskStruct *task = create_task(TASK_TRANSMIT_FIRMWARE_OK, 0, NULL);
						//     g_async_queue_push(CANReceiveMSGQueue, task);
						// }
					}
					else
					{
						perror("socketCANReceive failed");
					}
				}
			}
		}
		break;
		case RECEIVE_TASK_EXIT:
		{
			printf("RECEIVE_TASK_EXIT\n");
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
