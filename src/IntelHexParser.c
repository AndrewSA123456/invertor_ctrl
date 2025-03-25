#include "IntelHexParser.h"
#include "aes.h"

/////////////////////////////////////////////////////////////////////
// Функция: Intel-Hex разборщик
int IntelHexParser(char *file_name, ExtractedFirmwareStruct *extractedData)
{
	char line[MAX_LINE_LENGTH] = {0};
	FILE *file = fopen(file_name, "r");

	if (file == NULL)
	{
		perror("Failed to open file");
		return INTEL_HEX_PARSER_ERROR;
	}

	while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
	{
		if(IntelHexParser_ParseLine(line, strlen(line), extractedData) == INTEL_HEX_PARSER_ERROR)
		{
			printf("failed line: %s\n",line);
			fclose(file);
			return INTEL_HEX_PARSER_ERROR;
		}
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
		printf("line[0] != ':'\n");
		return INTEL_HEX_PARSER_ERROR;
	}

	uint32_t size_data, address, type;
	sscanf(line + 1, "%02x%04x%02x", &size_data, &address, &type);
	record.size_data = (uint8_t)size_data;
	record.address = (uint16_t)address;
	record.type = (uint8_t)type;
	// printf("Type: 0x%X Size: 0x%X Address: 0x%X\n", record.type, record.size_data, record.address);

	record.data = (uint8_t *)calloc(record.size_data, sizeof(uint8_t));

	if (record.data == NULL)
	{
		printf("record.data == NULL\n");
		return INTEL_HEX_PARSER_ERROR;
	}

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
		printf("checksum != 0\n");
		free(record.data);
		return INTEL_HEX_PARSER_ERROR;
	}

	switch (record.type)
	{
	case PROGRAMM_DATA:
	{
		if (prepare_program_data(&record, extractedData))
		{
			printf("Failed prepare_program_data(&record, extractedData)\n");
			free(record.data);
			return INTEL_HEX_PARSER_ERROR;
		}
	}
	break;
	case INTERRUPT_VECTOR_ADDRESS:
	{
		if (prepare_vectors_address(&record, extractedData))
		{
			printf("Failed prepare_vectors_address(&record, extractedData)\n");
			free(record.data);
			return INTEL_HEX_PARSER_ERROR;
		}
	}
	break;
	case START_PROGRAMM_ADDRESS:
	{
		if (prepare_programm_address(&record, extractedData))
		{
			printf("Failed prepare_programm_address(&record, extractedData)\n");
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
	if (intelHexData->size_data == 0 || intelHexData->data == NULL)
	{
		free(intelHexData->data);
		return INTEL_HEX_PARSER_ERROR;
	}
	uint32_t new_entry_start_mark;
	if (extractedData->program_data == NULL)
	{
		new_entry_start_mark = 0;
		extractedData->program_data_size = intelHexData->size_data;
		// printf("Program data size: %d\n", extractedData->program_data_size);
		extractedData->program_data = (uint8_t *)calloc(extractedData->program_data_size, sizeof(uint8_t));
	}
	else
	{
		new_entry_start_mark = extractedData->program_data_size;
		extractedData->program_data_size += intelHexData->size_data;
		// printf("Program data size: %d\n", extractedData->program_data_size);
		extractedData->program_data =
			(uint8_t *)realloc(extractedData->program_data, extractedData->program_data_size * sizeof(uint8_t));
	}

	if (extractedData->program_data == NULL)
	{
		free(intelHexData->data);
		return INTEL_HEX_PARSER_ERROR;
	}

	for (int i = 0; i < intelHexData->size_data; i++)
	{
		extractedData->program_data[new_entry_start_mark + i] = intelHexData->data[i];
		// printf("extractedData->program_data[%d] = 0x%X\n", new_entry_start_mark + i,
		// 	extractedData->program_data[new_entry_start_mark + i]);
	}
	return INTEL_HEX_PARSER_OK;
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
		// printf("Vectors address:	0x%X\n", extractedData->vectors_address);
	}
	else
	{
		printf("intelHexData->size_data != 2\n");
		return INTEL_HEX_PARSER_ERROR;
	}
	return INTEL_HEX_PARSER_OK;
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
		// printf("Programm address:	0x%X\n", extractedData->programm_address);
	}
	else
	{
		return INTEL_HEX_PARSER_ERROR;
	}
	return INTEL_HEX_PARSER_OK;
}