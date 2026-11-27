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
#include "app_it.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "task_sensor_mq_attribute.h"

/********************** macros and definitions *******************************/
#define SENSOR_MQ_CFG_QTY		(sizeof(task_sensor_mq_cfg_list)/sizeof(task_sensor_mq_cfg_t))
#define SENSOR_MQ_DTA_QTY		SENSOR_MQ_CFG_QTY

#define TICK_MAX 1000
#define RAFAGA_COUNT 10
#define SENSOR_MQ_NOISE_LVL 50

/********************** external data declaration ****************************/
extern volatile bool 		g_adc1_b_trig_new_conversion;
extern volatile uint32_t 	g_adc1_vector_leido[RAFAGA_COUNT];
extern volatile uint32_t 	g_adc1_sample_idx;
extern volatile bool 		g_adc2_b_trig_new_conversion;
extern volatile uint32_t 	g_adc2_vector_leido[RAFAGA_COUNT];
extern volatile uint32_t 	g_adc2_sample_idx;
extern ADC_HandleTypeDef 	hadc1;
extern ADC_HandleTypeDef 	hadc2;


/********************** internal data declaration ****************************/
const task_sensor_mq_cfg_t task_sensor_mq_cfg_list[] = {
		{ ID_SENSOR_MQ4,   &hadc1, TICK_MAX }, // MQ 4 (A0)
		{ ID_SENSOR_MQ135, &hadc2, TICK_MAX }  // MQ 135 (A1)
};

task_sensor_mq_dta_t task_sensor_mq_dta_list[SENSOR_MQ_DTA_QTY];

/********************** internal functions declaration ***********************/
void task_sensor_mq_statechart(uint32_t index);

/********************** internal data definition *****************************/
const char *p_task_sensor_mq 		= "Task Sensor MQ (Sensor MQ Statechart)";
const char *p_task_sensor_mq_ 		= "Non-Blocking Code";
const char *p_task_sensor_mq__ 		= "(Update by Time Code, period = 1mS)";



/********************** external functions definition ************************/
void task_sensor_mq_init(void *parameters)
{
	uint32_t index;
	task_sensor_mq_dta_t *p_task_sensor_mq_dta;
	const task_sensor_mq_cfg_t *p_task_sensor_mq_cfg;
	task_sensor_mq_st_t state;
	task_sensor_mq_ev_t event;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", GET_NAME(task_sensor_init), HAL_GetTick());
	LOGGER_INFO("   %s is a %s", GET_NAME(task_sensor), p_task_sensor_mq);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_sensor), p_task_sensor_mq_);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_sensor), p_task_sensor_mq__);

	for (index = 0; SENSOR_MQ_DTA_QTY > index; index++)
	{
		/* Update Task Sensor Data Pointer */
		p_task_sensor_mq_dta = &task_sensor_mq_dta_list[index];
		p_task_sensor_mq_cfg = &task_sensor_mq_cfg_list[index];

		/* Init & Print out: Index & Task execution FSM */
		state = ST_SENSOR_MQ_IDLE;
		event = EV_SENSOR_MQ_IDLE;

		if ( p_task_sensor_mq_cfg->identifier == ID_SENSOR_MQ4 ) {
			p_task_sensor_mq_dta->b_trig_new_conversion = &g_adc1_b_trig_new_conversion;
			p_task_sensor_mq_dta->vector_leido = &(g_adc1_vector_leido[0]);
			p_task_sensor_mq_dta->sample_idx   = &g_adc1_sample_idx;
		} else if ( p_task_sensor_mq_cfg->identifier == ID_SENSOR_MQ135 ) {
			p_task_sensor_mq_dta->b_trig_new_conversion = &g_adc2_b_trig_new_conversion;
			p_task_sensor_mq_dta->vector_leido = &(g_adc2_vector_leido[0]);
			p_task_sensor_mq_dta->sample_idx   = &g_adc2_sample_idx;
		}

		*(p_task_sensor_mq_dta->sample_idx) = 0;
		*(p_task_sensor_mq_dta->b_trig_new_conversion) = false;
		p_task_sensor_mq_dta->state 	  = state;
		p_task_sensor_mq_dta->event 	  = event;
		p_task_sensor_mq_dta->valor_leido = 0;

		LOGGER_INFO(" ");
		LOGGER_INFO("   %s = %lu   %s = %lu   %s = %lu",
				    GET_NAME(index), index,
					GET_NAME(state), (uint32_t)state,
					GET_NAME(event), (uint32_t)event);

	}
}

void task_sensor_mq_update(void *parameters)
{
	uint32_t index;

	for (index = 0; SENSOR_MQ_DTA_QTY > index; index++)
	{
		/* Run Task Statechart */
		task_sensor_mq_statechart(index);
	}
}

void task_sensor_mq_statechart(uint32_t index)
{
	const task_sensor_mq_cfg_t *p_task_sensor_mq_cfg;
	task_sensor_mq_dta_t *p_task_sensor_mq_dta;


		/* Update Task Sensor Configuration & Data Pointer */
		p_task_sensor_mq_cfg = &task_sensor_mq_cfg_list[index];
		p_task_sensor_mq_dta = &task_sensor_mq_dta_list[index];

		switch (p_task_sensor_mq_dta->state)
		{
			case ST_SENSOR_MQ_IDLE:
				p_task_sensor_mq_dta->state = ST_SENSOR_MQ_WAITING;
				p_task_sensor_mq_dta->tick = p_task_sensor_mq_cfg->tick_max;
				break;

			case ST_SENSOR_MQ_WAITING:

				p_task_sensor_mq_dta->tick--;
				if (p_task_sensor_mq_dta->tick == 0)
				{
					p_task_sensor_mq_dta->state = ST_SENSOR_MQ_SENSING;
				}

				break;

			case ST_SENSOR_MQ_SENSING:

				bool b_done = false;

				if ( *(p_task_sensor_mq_dta->sample_idx) >= RAFAGA_COUNT) {
					b_done = true;
					goto test3_tick_end;
				}

				/* start of first conversion */
				if (0 == *(p_task_sensor_mq_dta->sample_idx)) {
					*(p_task_sensor_mq_dta->b_trig_new_conversion) = true;
				}

				if (*(p_task_sensor_mq_dta->b_trig_new_conversion)) {
					*(p_task_sensor_mq_dta->b_trig_new_conversion) = false;
					HAL_ADC_Start_IT(p_task_sensor_mq_cfg->hadc);
				}

test3_tick_end:
				if (b_done) {
					uint32_t acum = 0;
					for (uint32_t i = 0;i < RAFAGA_COUNT; i++) {
						acum += p_task_sensor_mq_dta->vector_leido[i];
					}
					if ( acum/RAFAGA_COUNT < SENSOR_MQ_NOISE_LVL ) { // error de sensado
						put_event_task_system(EV_SYS_SENSOR_ERROR, NULL );
					} else {
						if ( p_task_sensor_mq_cfg->identifier == ID_SENSOR_MQ4 ) {
							p_task_sensor_mq_dta->valor_leido = acum/RAFAGA_COUNT;
							put_event_task_system(EV_SYS_GAS_INFO, &(p_task_sensor_mq_dta->valor_leido) );
						} else if ( p_task_sensor_mq_cfg->identifier == ID_SENSOR_MQ135 ) {
							//LOGGER_LOG("MQ-135: %d\n", (int)(((((float)acum)/RAFAGA_COUNT)/3300)*100) );
							p_task_sensor_mq_dta->valor_leido = acum/RAFAGA_COUNT;
							put_event_task_system(EV_SYS_CO2_INFO, &(p_task_sensor_mq_dta->valor_leido) );
						}
					}
					*(p_task_sensor_mq_dta->sample_idx) = 0;
					p_task_sensor_mq_dta->state = ST_SENSOR_MQ_WAITING;
					p_task_sensor_mq_dta->tick = p_task_sensor_mq_cfg->tick_max;
				}

				break;

			default:
				p_task_sensor_mq_dta->tick  = p_task_sensor_mq_cfg->tick_max;
				p_task_sensor_mq_dta->state = ST_SENSOR_MQ_IDLE;
				p_task_sensor_mq_dta->event = EV_SENSOR_MQ_IDLE;

			break;
		}
}

/********************** end of file ******************************************/
