#ifndef __INTELHEXPARSER_H
#define __INTELHEXPARSER_H

#include "globals.h"
typedef struct
{
	uint32_t vectors_address;
	uint32_t programm_address;
	uint8_t *program_data;
	uint32_t program_data_size;
} ExtractedFirmwareStruct;
/////////////////////////////////////////////////////////////////////
// Функция: Intel-Hex разборщик
int IntelHexParser(char *file_name,
	ExtractedFirmwareStruct *extractedData);
//-----DEFINES
#define MAX_LINE_LENGTH 256
enum ParserStatus
{
	INTEL_HEX_PARSER_OK,
	INTEL_HEX_PARSER_ERROR
};
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Intel-Hex разборщик строки
int IntelHexParser_ParseLine(char *line,
							 size_t len,
							 ExtractedFirmwareStruct *extractedData);
//-----DEFINES
enum IntelHexRecordType
{
	PROGRAMM_DATA = 0,
	EndOfFileRecord = 1,
	ExtendedSegmentAddressRecord = 2,
	StartSegmentAddressRecord = 3,
	INTERRUPT_VECTOR_ADDRESS = 4,
	START_PROGRAMM_ADDRESS = 5
};
//-----STRUCTS
typedef struct
{
	uint8_t size_data;
	uint16_t address;
	uint8_t type;
	uint8_t *data;
	uint8_t checksum;
} IntelHexRecord;
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Подготовка данных программы
int prepare_program_data(IntelHexRecord *intelHexData,
						 ExtractedFirmwareStruct *extractedData);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Подготовка адреса векторов прерывания
int prepare_vectors_address(IntelHexRecord *intelHexData,
							ExtractedFirmwareStruct *extractedData);
//-----DEFINES
#define VECTORS_ADDRESS_DATA_OFFSET 3
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Подготовка адреса старта программы
int prepare_programm_address(IntelHexRecord *intelHexData,
							 ExtractedFirmwareStruct *extractedData);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция:
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
#endif // __INTELHEXPARSER_H