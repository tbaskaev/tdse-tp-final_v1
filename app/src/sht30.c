#include "main.h"
#include "sht30.h"

/* macros */

/* external variables */

/* internal variables */

/* internal functions */
static uint8_t SHT30_CheckCRC(uint8_t *data, uint8_t len) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

/* interface */
void SHT30_Init(SHT30_handle_t *handle, I2C_HandleTypeDef *hi2c) {
	handle->hi2c = hi2c;
	handle->receive_complete = false;
	handle->transmit_complete = false;
	handle->error = false;
	handle->tx_buf[0] = SHT30_CMD_MSB;
	handle->tx_buf[1] = SHT30_CMD_LSB;
}

SHT30_response_t SHT30_StartMeasurement(SHT30_handle_t *handle) {
	handle->transmit_complete = false;
	handle->error             = false;

	handle->tx_buf[0] = SHT30_CMD_MSB;
	handle->tx_buf[1] = SHT30_CMD_LSB;
	if (HAL_I2C_Master_Transmit_IT(handle->hi2c, SHT30_I2C_ADDR, handle->tx_buf, 2) != HAL_OK) {
		return SHT30_ERROR;
	}
	return SHT30_OK;
}

SHT30_response_t SHT30_StartReceive(SHT30_handle_t *handle) {
	handle->receive_complete = false;
	handle->error = false;

	if ( HAL_I2C_Master_Receive_IT(handle->hi2c, SHT30_I2C_ADDR, handle->rx_buf, 6) != HAL_OK ) {
		return SHT30_ERROR;
	}
	return SHT30_OK;
}

SHT30_response_t SHT30_GetData(SHT30_handle_t *handle, SHT30_data_t *out_data) {
	if (SHT30_CheckCRC(&handle->rx_buf[0], 2) == handle->rx_buf[2] &&
	                SHT30_CheckCRC(&handle->rx_buf[3], 2) == handle->rx_buf[5]) {
		uint16_t raw_temp = (handle->rx_buf[0] << 8) | handle->rx_buf[1];
		uint16_t raw_hum  = (handle->rx_buf[3] << 8) | handle->rx_buf[4];

		// Fórmulas oficiales del datasheet
		out_data->temperatura 	 = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
		out_data->humedad_relativa = 100.0f * ((float)raw_hum / 65535.0f);

		return SHT30_OK;
	} else {
		return SHT30_ERROR;
	}
}

void SHT30_Abort(SHT30_handle_t *handle) {
	HAL_I2C_Master_Abort_IT(handle->hi2c, SHT30_I2C_ADDR);
}

/* Callbacks */
void SHT30_TxCpltCallback(I2C_HandleTypeDef *hi2c, SHT30_handle_t *hsht30) {
	if ( hi2c == hsht30->hi2c) {
		hsht30->transmit_complete = true;
	}
}

void SHT30_RxCpltCallback(I2C_HandleTypeDef *hi2c, SHT30_handle_t *hsht30) {
    if ( hi2c == hsht30->hi2c ) {
    	hsht30->receive_complete = true;
    }
}

void SHT30_ErrorCallback(I2C_HandleTypeDef *hi2c, SHT30_handle_t *hsht30) {
	if ( hi2c == hsht30->hi2c ) {
		hsht30->error = true;
	}
}
