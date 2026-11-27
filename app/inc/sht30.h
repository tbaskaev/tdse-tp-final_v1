#ifndef INC_SHT30_H_
#define INC_SHT30_H_

#include "main.h"
#include <stdbool.h>

#define SHT30_I2C_ADDR      (0x44 << 1)

// Command: Single Shot, High Repeatability, Clock Stretching Disabled
#define SHT30_CMD_MSB       0x24
#define SHT30_CMD_LSB       0x00

// structs
typedef struct {
	float temperatura;
	float humedad_relativa;
} SHT30_data_t;

typedef enum {
	SHT30_OK, SHT30_ERROR
} SHT30_response_t;

typedef struct {
	I2C_HandleTypeDef  *hi2c;
	uint8_t 			rx_buf[6];
	uint8_t 			tx_buf[2];
	volatile bool 		transmit_complete;
	volatile bool 		receive_complete;
	volatile bool 		error;
} SHT30_handle_t;

// Interface por interrupciones
void 			 SHT30_Init(SHT30_handle_t *handle, I2C_HandleTypeDef *hi2c);
SHT30_response_t SHT30_StartMeasurement(SHT30_handle_t *handle);
SHT30_response_t SHT30_StartReceive(SHT30_handle_t *handle);
SHT30_response_t SHT30_GetData(SHT30_handle_t *handle, SHT30_data_t *out_data);
void			 SHT30_Abort(SHT30_handle_t *handle);


// Callbacks para llamar desde las interrupciones de HAL
void SHT30_TxCpltCallback(I2C_HandleTypeDef *hi2c, SHT30_handle_t *hsht30);
void SHT30_RxCpltCallback(I2C_HandleTypeDef *hi2c, SHT30_handle_t *hsht30);
void SHT30_ErrorCallback(I2C_HandleTypeDef *hi2c, SHT30_handle_t *hsht30);

#endif
