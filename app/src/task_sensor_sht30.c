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
#include "app.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "task_sensor_sht30_attribute.h"

/********************** macros and definitions *******************************/
#define SENSOR_SHT30_CFG_QTY		(sizeof(task_sensor_sht30_cfg_list)/sizeof(task_sensor_sht30_cfg_t))
#define SENSOR_SHT30_DTA_QTY		SENSOR_SHT30_CFG_QTY

/********************** internal data declaration ****************************/
SHT30_handle_t hSHT30;


const task_sensor_sht30_cfg_t task_sensor_sht30_cfg_list[] = {
	{ SENSOR_SHT30_TICK_MAX, &hSHT30 }
};

task_sensor_sht30_dta_t task_sensor_sht30_dta_list[SENSOR_SHT30_DTA_QTY];


/********************** internal functions declaration ***********************/
void task_sensor_sht30_statechart(uint32_t index);

/********************** internal data definition *****************************/
const char *p_task_sensor_sht30 		= "Task Sensor SHT30 (Sensor Statechart)";
const char *p_task_sensor_sht30_ 		= "Non-Blocking Code";
const char *p_task_sensor_sht30__ 		= "(Update by Time Code, period = 1mS)";

/********************** external data declaration ****************************/
extern I2C_HandleTypeDef hi2c1;

/********************** external functions definition ************************/
void task_sensor_sht30_init(void *parameters)
{
	uint32_t index;
	task_sensor_sht30_dta_t *p_task_sensor_sht30_dta;
	const task_sensor_sht30_cfg_t *p_task_sensor_sht30_cfg;
	task_sensor_sht30_st_t state;
	task_sensor_sht30_ev_t event;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", GET_NAME(task_sensor_sht30_init), HAL_GetTick());
	LOGGER_INFO("   %s is a %s", GET_NAME(task_sensor_sht30), p_task_sensor_sht30);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_sensor_sht30), p_task_sensor_sht30_);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_sensor_sht30), p_task_sensor_sht30__);

	for (index = 0; SENSOR_SHT30_DTA_QTY > index; index++)
	{
		/* Update Task Sensor Data Pointer */
		p_task_sensor_sht30_dta = &task_sensor_sht30_dta_list[index];
		p_task_sensor_sht30_cfg = &task_sensor_sht30_cfg_list[index];

		/* Init & Print out: Index & Task execution FSM */
		state = ST_SENSOR_SHT30_IDLE;
		event = EV_SENSOR_SHT30_IDLE;

		p_task_sensor_sht30_dta->state = state;
		p_task_sensor_sht30_dta->event = event;
		p_task_sensor_sht30_dta->error_retry_count = 0;

		SHT30_Init( p_task_sensor_sht30_cfg->hSHT30, &hi2c1 );

		LOGGER_INFO(" ");
		LOGGER_INFO("   %s = %lu   %s = %lu   %s = %lu",
				    GET_NAME(index), index,
					GET_NAME(state), (uint32_t)state,
					GET_NAME(event), (uint32_t)event);
	}
}

void task_sensor_sht30_update(void *parameters)
{
	uint32_t index;

	for (index = 0; SENSOR_SHT30_DTA_QTY > index; index++)
	{
		/* Run Task Statechart */
		task_sensor_sht30_statechart(index);
	}
}

void task_sensor_sht30_statechart(uint32_t index)
{
	const task_sensor_sht30_cfg_t *p_task_sensor_sht30_cfg;
	task_sensor_sht30_dta_t *p_task_sensor_sht30_dta;

	/* Update Task Sensor Configuration & Data Pointer */
	p_task_sensor_sht30_cfg = &task_sensor_sht30_cfg_list[index];
	p_task_sensor_sht30_dta = &task_sensor_sht30_dta_list[index];

	switch (p_task_sensor_sht30_dta->state)
	{
		case ST_SENSOR_SHT30_IDLE:
			p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_WAITING;
			p_task_sensor_sht30_dta->tick = p_task_sensor_sht30_cfg->tick_max;
			//p_task_sensor_sht30_dta->error_retry_count = 0;
			break;

		case ST_SENSOR_SHT30_WAITING:
			p_task_sensor_sht30_dta->tick--;
			if ( p_task_sensor_sht30_dta->tick == 0 ) {
				if ( SHT30_StartMeasurement( p_task_sensor_sht30_cfg->hSHT30 ) == SHT30_OK) {
					p_task_sensor_sht30_dta->tick = SENSOR_SHT30_I2C_TIMEOUT;
					p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_TRANSMITING;
				} else { // error
					put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
					p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
				}
			}
			break;

		case ST_SENSOR_SHT30_TRANSMITING:
			if ( p_task_sensor_sht30_cfg->hSHT30->transmit_complete ) {
				p_task_sensor_sht30_cfg->hSHT30->transmit_complete = false;
				p_task_sensor_sht30_dta->tick = SENSOR_SHT30_WAIT_RX_INTERVAL;
				p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_SENSING;
				p_task_sensor_sht30_dta->error_retry_count = 0;
			} else if (p_task_sensor_sht30_cfg->hSHT30->error ) {
				p_task_sensor_sht30_cfg->hSHT30->error = false; // error
				if ( p_task_sensor_sht30_dta->error_retry_count++ == SENSOR_SHT30_RETRY_MAX ) {
					put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
				}
				p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
			} else if ( --p_task_sensor_sht30_dta->tick == 0 ) {
				SHT30_Abort(p_task_sensor_sht30_cfg->hSHT30);
				if ( p_task_sensor_sht30_dta->error_retry_count++ == SENSOR_SHT30_RETRY_MAX ) {
					put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
				}
				p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
			}
			break;

		case ST_SENSOR_SHT30_SENSING: // wait 20 ms for sht30 to get data
			p_task_sensor_sht30_dta->tick--;
			if ( p_task_sensor_sht30_dta->tick == 0 ) {
				if ( SHT30_StartReceive( p_task_sensor_sht30_cfg->hSHT30 ) == SHT30_OK ) {
					p_task_sensor_sht30_dta->tick = SENSOR_SHT30_I2C_TIMEOUT;
					p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_RECEIVING;
					p_task_sensor_sht30_dta->error_retry_count = 0;
				} else { // error
					if ( p_task_sensor_sht30_dta->error_retry_count++ == SENSOR_SHT30_RETRY_MAX ) {
						put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
					}
					p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
				}
			}
			break;
		case ST_SENSOR_SHT30_RECEIVING:
			if ( p_task_sensor_sht30_cfg->hSHT30->receive_complete ) {
				p_task_sensor_sht30_cfg->hSHT30->receive_complete = false;
				p_task_sensor_sht30_dta->error_retry_count = 0;

				if ( SHT30_GetData( p_task_sensor_sht30_cfg->hSHT30, &(p_task_sensor_sht30_dta->datos) ) == SHT30_OK ) {
					// send data to system
					put_event_task_system(EV_SYS_TEMP_HUM_INFO, &(p_task_sensor_sht30_dta->datos) );
					//LOGGER_LOG("TEMP: %d\n", (int)p_task_sensor_sht30_dta->datos.temperatura );
					//LOGGER_LOG("HUME: %d\n\n", (int)p_task_sensor_sht30_dta->datos.humedad_relativa );
				}

				p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
			} else if (p_task_sensor_sht30_cfg->hSHT30->error ) { // error por interrupt
				p_task_sensor_sht30_cfg->hSHT30->error = false;
				if ( p_task_sensor_sht30_dta->error_retry_count++ == SENSOR_SHT30_RETRY_MAX ) {
					put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
				}
				p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
			} else if ( --p_task_sensor_sht30_dta->tick == 0 ) { // error por timeout
				SHT30_Abort(p_task_sensor_sht30_cfg->hSHT30);
				if ( p_task_sensor_sht30_dta->error_retry_count++ == SENSOR_SHT30_RETRY_MAX ) {
					put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
				}
				p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
			}
			break;

		default:

			p_task_sensor_sht30_dta->tick  = p_task_sensor_sht30_cfg->tick_max;
			//p_task_sensor_sht30_dta->error_retry_count = 0;
			p_task_sensor_sht30_dta->state = ST_SENSOR_SHT30_IDLE;
			p_task_sensor_sht30_dta->event = EV_SENSOR_SHT30_IDLE;

			break;
	}
}

/********************** end of file ******************************************/
