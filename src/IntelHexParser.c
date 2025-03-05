#include "IntelHexParser.h"
#include "aes.h"

/////////////////////////////////////////////////////////////////////
// Функция: Intel-Hex разборщик
int IntelHexParser(char *file_name, ExtractedFirmwareStruct *extractedData)
{
	char line[MAX_LINE_LENGTH] = {0};
	FILE *file = fopen("stm32g474ret6.hex", "r");
	if (file == NULL)
	{
		perror("Failed to open file");
		return INTEL_HEX_PARSER_ERROR;
	}
	while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
	{
		IntelHexParser_ParseLine(line, strlen(line), extractedData);
	}

	printf("Program data size: %d\n", extractedData->program_data_size);
	fclose(file);
	return INTEL_HEX_PARSER_OK;
}
/////////////////////////////////////////////////////////////////////
// Функция: Intel-Hex разборщик строки
int IntelHexParser_ParseLine(char *line, size_t len, ExtractedFirmwareStruct *extractedData)
{
	IntelHexRecord record;

	if (line[0] != ':')
	{
		return INTEL_HEX_PARSER_ERROR;
	}
	uint32_t size_data, address, type;
	sscanf(line + 1, "%02x%04x%02x", &size_data, &address, &type);
	record.size_data = (uint8_t)size_data;
	record.address = (uint16_t)address;
	record.type = (uint8_t)type;
	// printf("Type: 0x%X Size: 0x%X Address: 0x%X\n", record.type, record.size_data, record.address);

	record.data = (uint8_t *)calloc(record.size_data, sizeof(uint8_t));

	uint8_t checksum = record.size_data +
					   ((record.address >> 8) & 0xFF) +
					   (record.address & 0xFF) +
					   record.type;

	for (int i = 0; i < record.size_data; i++)
	{
		uint32_t byte;
		sscanf(line + 9 + i * 2, "%02x", &byte);
		record.data[i] = (uint8_t)byte;
		// printf("0x%X ", record.data[i]);
		checksum += byte;
	}
	// printf("\n");
	uint32_t file_checksum = 0;
	sscanf(line + 9 + record.size_data * 2, "%02x", &file_checksum);
	record.checksum = (uint8_t)file_checksum;
	// printf("Checksum: 0x%X\n", record.checksum);
	checksum = (checksum + file_checksum) & 0xFF;

	if (checksum != 0)
	{
		free(record.data);
		return INTEL_HEX_PARSER_ERROR;
	}

	switch (record.type)
	{
	case PROGRAMM_DATA:
	{
		if (prepare_program_data(&record, extractedData))
		{
			free(record.data);
			return INTEL_HEX_PARSER_ERROR;
		}
	}
	break;
	case INTERRUPT_VECTOR_ADDRESS:
	{
		if (prepare_vectors_address(&record, extractedData))
		{
			free(record.data);
			return INTEL_HEX_PARSER_ERROR;
		}
	}
	break;
	case START_PROGRAMM_ADDRESS:
	{
		if (prepare_programm_address(&record, extractedData))
		{
			free(record.data);
			return INTEL_HEX_PARSER_ERROR;
		}
	}
	break;
	}
	free(record.data);
	return INTEL_HEX_PARSER_OK;
}
/////////////////////////////////////////////////////////////////////
// Функция: Подготовка данных программы
int prepare_program_data(IntelHexRecord *intelHexData,
						 ExtractedFirmwareStruct *extractedData)
{
	if (intelHexData->size_data == 0)
	{
		free(intelHexData->data);
		return INTEL_HEX_PARSER_ERROR;
	}
	if (extractedData->program_data == NULL)
	{
		extractedData->program_data_size = intelHexData->size_data;
		// printf("Program data size: %d\n", extractedData->program_data_size);
		extractedData->program_data = (uint8_t *)calloc(extractedData->program_data_size, sizeof(uint8_t));
	}
	else
	{
		extractedData->program_data_size += intelHexData->size_data;
		// printf("Program data size: %d\n", extractedData->program_data_size);
		extractedData->program_data =
			(uint8_t *)realloc(extractedData->program_data, extractedData->program_data_size * sizeof(uint8_t));
	}
	for (int i = 0; i < intelHexData->size_data; i++)
	{
		extractedData->program_data[intelHexData->address + i] = intelHexData->data[i];
	}
}
/////////////////////////////////////////////////////////////////////
// Функция: Подготовка адреса векторов прерывания
int prepare_vectors_address(IntelHexRecord *intelHexData,
							ExtractedFirmwareStruct *extractedData)
{
	if (intelHexData->size_data == 2)
	{
		for (int i = 0; i < intelHexData->size_data; i++)
		{
			extractedData->vectors_address |=
				intelHexData->data[i] << (8 * (VECTORS_ADDRESS_DATA_OFFSET - i));
		}
		extractedData->vectors_address |= (((uint32_t)intelHexData->address) & 0xFFFF);
		printf("Vectors address:	0x%X\n", extractedData->vectors_address);
	}
	else
	{
		return INTEL_HEX_PARSER_ERROR;
	}
}
/////////////////////////////////////////////////////////////////////
// Функция: Подготовка адреса старта программы
int prepare_programm_address(IntelHexRecord *intelHexData,
							 ExtractedFirmwareStruct *extractedData)
{
	if (intelHexData->size_data == 4)
	{
		for (int i = 0; i < intelHexData->size_data; i++)
		{
			extractedData->programm_address |=
				intelHexData->data[i] << (8 * (intelHexData->size_data - 1 - i));
		}
		printf("Programm address:	0x%X\n", extractedData->programm_address);
	}
	else
	{
		return INTEL_HEX_PARSER_ERROR;
	}
}