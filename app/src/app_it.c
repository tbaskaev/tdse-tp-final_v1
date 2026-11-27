/*
 * Copyright (c) 2026 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "sht30.h"

#include "task_sensor_mq_attribute.h"
#include "task_sensor_sht30_attribute.h"

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
volatile uint32_t g_app_tick_cnt;
extern I2C_HandleTypeDef hi2c1;

/* MQ vars */
// MQ 4
volatile uint32_t 		 g_adc1_vector_leido[RAFAGA_COUNT];
volatile uint32_t 		 g_adc1_sample_idx;
volatile bool 	  		 g_adc1_b_trig_new_conversion;
extern ADC_HandleTypeDef hadc1;
// MQ 135
volatile uint32_t 		 g_adc2_vector_leido[RAFAGA_COUNT];
volatile uint32_t 		 g_adc2_sample_idx;
volatile bool 	  		 g_adc2_b_trig_new_conversion;
extern ADC_HandleTypeDef hadc2;




/********************** external functions definition ************************/
void app_it_init(void)
{
	/* Protect shared resource */
	__asm("CPSID i");	/* disable interrupts */
	/* Init Tick Counter */
	g_app_tick_cnt = ZERO;
    __asm("CPSIE i");	/* enable interrupts */
}

void HAL_SYSTICK_Callback(void)
{
	/* Update Tick Counter */
	g_app_tick_cnt++;
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// Check which version of the gpio triggered this callback
	if (GPIO_Pin == BTN_A_PIN)
	{
		/* Work to be done. */
	}
}

/* ADC callback */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {

	if ( hadc == &hadc1 ) {
		uint32_t valor = HAL_ADC_GetValue(&hadc1);
		g_adc1_vector_leido[g_adc1_sample_idx++] = valor;
		if (g_adc1_sample_idx<RAFAGA_COUNT) {
			g_adc1_b_trig_new_conversion = true;
		}
	} else if ( hadc == &hadc2 ) {
		uint32_t valor = HAL_ADC_GetValue(&hadc2);
		g_adc2_vector_leido[g_adc2_sample_idx++] = valor;
		if (g_adc2_sample_idx<RAFAGA_COUNT) {
			g_adc2_b_trig_new_conversion = true;
		}
	}
}


/* I2C callbacks */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) { // transmit complete
	SHT30_TxCpltCallback( hi2c, &hSHT30 );
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SHT30_RxCpltCallback(hi2c, &hSHT30);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    SHT30_ErrorCallback(hi2c, &hSHT30);
}

/********************** end of file ******************************************/
