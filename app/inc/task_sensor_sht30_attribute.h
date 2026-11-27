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

#ifndef TASK_SENSOR_SHT30_ATTRIBUTE_H_
#define TASK_SENSOR_SHT30_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include "sht30.h"

/********************** macros ***********************************************/
#define SENSOR_SHT30_TICK_MAX 1000
#define SENSOR_SHT30_WAIT_RX_INTERVAL 20
#define SENSOR_SHT30_I2C_TIMEOUT 10

/********************** typedef **********************************************/
/* Events to excite Task Sensor */
typedef enum task_sensor_sht30_ev { EV_SENSOR_SHT30_IDLE } task_sensor_sht30_ev_t;

/* States of Task Sensor */
typedef enum task_sensor_sht30_st { ST_SENSOR_SHT30_IDLE,
									ST_SENSOR_SHT30_TRANSMITING,
							        ST_SENSOR_SHT30_WAITING,
									ST_SENSOR_SHT30_SENSING,
									ST_SENSOR_SHT30_RECEIVING } task_sensor_sht30_st_t;

typedef struct
{
	uint32_t			tick_max;
	SHT30_handle_t 		*hSHT30;
} task_sensor_sht30_cfg_t;

typedef struct
{
	uint32_t				tick;
	task_sensor_sht30_st_t	state;
	task_sensor_sht30_ev_t	event;
	SHT30_data_t  			datos;
} task_sensor_sht30_dta_t;

/********************** external data declaration ****************************/
extern task_sensor_sht30_dta_t task_sensor_sht30_dta_list[];
extern SHT30_handle_t hSHT30;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_SENSOR_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
