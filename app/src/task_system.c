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

#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
#include "task_display_attribute.h"
#include "task_display_interface.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "sht30.h"

/********************** macros and definitions *******************************/
#define DEL_SYS_MIN			0ul
#define DEL_SYS_MED			250ul
#define DEL_SYS_MAX			500ul

/* Modes to excite Task System */
typedef enum task_system_mode {NORMAL, SETUP, FALLA} task_system_mode_t;

#define SYSTEM_DTA_QTY	3

/********************** internal data declaration ****************************/
task_system_dta_t task_system_dta_list[SYSTEM_DTA_QTY];

/********************** internal functions declaration ***********************/
void task_system_normal_statechart(void);
void task_system_setup_statechart(void);
void task_system_falla_statechart(void);


void task_system_set_mode(task_system_mode_t);

/********************** internal data definition *****************************/
const char *p_task_system 		= "Task System (System Statechart)";
const char *p_task_system_ 		= "Non-Blocking Code";
const char *p_task_system__ 	= "(Update by Time Code, period = 1mS)";

/********************** external data declaration ****************************/
task_system_mode_t g_task_system_mode;

/********************** private functions definition  ************************/

/********************** external functions definition ************************/
void task_system_init(void *parameters)
{
	uint32_t index;
	task_system_dta_t 	*p_task_system_dta;
	task_system_st_t	state;
	task_system_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", GET_NAME(task_system_init), HAL_GetTick());
	LOGGER_INFO("   %s is a %s", GET_NAME(task_system), p_task_system);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_system), p_task_system_);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_system), p_task_system__);

	init_event_task_system();

	task_system_set_mode(NORMAL);

	for (index = 0; SYSTEM_DTA_QTY > index; index++)
	{
		/* Update Task System Data Pointer */
		p_task_system_dta = &task_system_dta_list[index];

		/* Init & Print out: Task execution FSM */
		state = ST_SYS_IDLE;
		p_task_system_dta->state = state;

		event.id = EV_SYS_IDLE,
		event.data = NULL;
		p_task_system_dta->event = event;

		p_task_system_dta->period_actuator_tx = SYSTEM_DEFAULT_PERIOD;
		p_task_system_dta->temperatura 		  = 0;
		p_task_system_dta->humedad 			  = 0;
		p_task_system_dta->gas     			  = 0;
		p_task_system_dta->co2 				  = 0;
		p_task_system_dta->variable_control   = &(p_task_system_dta->SYSTEM_DEFAULT_CONTROL_VAR);
		p_task_system_dta->umbral   		  = SYSTEM_DEFAULT_UMBRAL;
		p_task_system_dta->umbral_superado 	  = false;

		b_event = false;
		p_task_system_dta->flag = b_event;

		LOGGER_INFO(" ");
		LOGGER_INFO("   %s = %lu   %s = %lu   %s = %s",
					GET_NAME(state), (uint32_t)state,
					GET_NAME(event), (uint32_t)event.id,
					GET_NAME(b_event), (b_event ? "true" : "false"));
	}

	task_system_set_mode(NORMAL);
}

void task_system_update(void *parameters)
{
	/* Run Task Statechart */
	switch (g_task_system_mode)
	{
		case NORMAL:

			task_system_normal_statechart();

			break;

		case SETUP:

			task_system_setup_statechart();

			break;

		case FALLA:

			task_system_falla_statechart();

			break;

		default:

			task_system_set_mode(NORMAL);

			break;
		}
}

void task_system_normal_statechart(void)
{
	task_system_dta_t *p_task_system_dta;

	/* Update Task System Data Pointer */
	p_task_system_dta = &task_system_dta_list[NORMAL];

	if (true == any_event_task_system())
	{
		p_task_system_dta->flag = true;
		p_task_system_dta->event = get_event_task_system();
	}

	switch (p_task_system_dta->state)
	{
		case ST_SYS_IDLE:
			p_task_system_dta->state = ST_SYS_SENSING;
			p_task_system_dta->tick = p_task_system_dta->period_actuator_tx;
			// TODO: turn on green LED
			break;

		case ST_SYS_SENSING:
			p_task_system_dta->tick--;
			if ( true == p_task_system_dta->flag )
			{
				if ( EV_SYS_TEMP_HUM_INFO == p_task_system_dta->event.id ) {
					p_task_system_dta->flag = false;
					SHT30_data_t* data = (SHT30_data_t*)(p_task_system_dta->event.data);

					p_task_system_dta->temperatura = data->temperatura;
					p_task_system_dta->humedad = data->humedad_relativa;
				} else if ( EV_SYS_CO2_INFO == p_task_system_dta->event.id  ) {
					p_task_system_dta->flag = false;
					uint32_t* data = (uint32_t*)(p_task_system_dta->event.data);

					p_task_system_dta->co2 = (((float)(*data))/3300) * 100;
				} else if ( EV_SYS_GAS_INFO == p_task_system_dta->event.id  ) {
					p_task_system_dta->flag = false;
					uint32_t* data = (uint32_t*)(p_task_system_dta->event.data);

					p_task_system_dta->gas = (((float)(*data))/3300) * 100;
				} else if ( EV_SYS_SENSOR_ERROR == p_task_system_dta->event.id ) {
					task_system_set_mode(FALLA);
				}
			}
			if ( p_task_system_dta->tick == 0 ) {
				//TODO: send to actuator
				LOGGER_LOG("TEMP: %d\n", (int)(p_task_system_dta->temperatura) );
				LOGGER_LOG("HUME: %d\n", (int)(p_task_system_dta->humedad) );
				LOGGER_LOG("CO2 : %d%s\n", (int)(p_task_system_dta->co2), "%%");
				LOGGER_LOG("GAS : %d%s\n\n", (int)(p_task_system_dta->gas), "%%");
				p_task_system_dta->tick = p_task_system_dta->period_actuator_tx;
			}

			if ( *(p_task_system_dta->variable_control) >= p_task_system_dta->umbral && !p_task_system_dta->umbral_superado) {
				// TODO: send signal to actuator:
				// - Turn on PWN LED
				// - Buzzer
				p_task_system_dta->umbral_superado = true;
				LOGGER_LOG("\n\n------ UMBRAL SUPERADO ------\n\n");
			}
			if ( p_task_system_dta->umbral_superado && *(p_task_system_dta->variable_control) < p_task_system_dta->umbral ) {
				p_task_system_dta->umbral_superado = false;
				LOGGER_LOG("\n\n------ CRISIS EVITADA ------\n\n");
			}


			break;
		default:
			p_task_system_dta->state = ST_SYS_IDLE;
			p_task_system_dta->event.id = EV_SYS_IDLE;

			break;
	}
}


void task_system_falla_statechart(void)
{
	task_system_dta_t *p_task_system_dta;

	/* Update Task System Data Pointer */
	p_task_system_dta = &task_system_dta_list[SETUP];

	if (true == any_event_task_system())
	{
		p_task_system_dta->flag = true;
		p_task_system_dta->event = get_event_task_system();
	}

	switch (p_task_system_dta->state)
	{
		case ST_SYS_IDLE:
			/* TODO: Send Error To App via Bluetooth */
			// Prender LED Amarillo
			LOGGER_LOG("MODO ERROR\n");

			break;

		default:

			p_task_system_dta->tick  = DEL_SYS_MIN;
			p_task_system_dta->state = ST_SYS_IDLE;
			p_task_system_dta->event.id = EV_SYS_IDLE;
			p_task_system_dta->flag = false;

			break;
	}
}


void task_system_setup_statechart(void)
{
	task_system_dta_t *p_task_system_dta;

	/* Update Task System Data Pointer */
	p_task_system_dta = &task_system_dta_list[SETUP];

	if (true == any_event_task_system())
	{
		p_task_system_dta->flag = true;
		p_task_system_dta->event = get_event_task_system();
	}

	switch (p_task_system_dta->state)
	{/*
		case ST_SYS_IDLE:

			if ((true == p_task_system_dta->flag) && (EV_SYS_BTN_A == p_task_system_dta->event))
			{
				p_task_system_dta->flag = false;
				put_event_task_actuator(EV_LED_ACTIVE, ID_LED_A);
				p_task_system_dta->state = ST_SYS_ACTIVE;

				put_event_task_display(0, 0, "task_system_mode");
				put_event_task_display(0, 1, " NORMAL         ");

				task_system_set_mode(NORMAL);
			}

			break;

		case ST_SYS_ACTIVE:

			if ((true == p_task_system_dta->flag) && (EV_SYS_IDLE == p_task_system_dta->event))
			{
				p_task_system_dta->flag = false;
				put_event_task_actuator(EV_LED_IDLE, ID_LED_A);
				p_task_system_dta->state = ST_SYS_IDLE;
			}

			break;
*/
		default:

			p_task_system_dta->tick  = DEL_SYS_MIN;
			//p_task_system_dta->state = ST_SYS_IDLE;
			p_task_system_dta->event.id = EV_SYS_IDLE;
			p_task_system_dta->flag = false;

			break;
	}
}

void task_system_set_mode(task_system_mode_t task_system_mode)
{
	g_task_system_mode = task_system_mode;
}

/********************** end of file ******************************************/
