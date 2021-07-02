/*
 * Copyright (c) 2019 G.camp,
 *
 * Contact: Jin Seog Bang <seog814@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tizen.h>
#include <service_app.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <Ecore.h>

#include "st_things.h"
#include "log.h"
#include "sensor-data.h"
#include "resource.h"

#define SENSOR_GATHER_INTERVAL (1.0f)


typedef struct app_data_s {
	Ecore_Timer *getter_sw;
	sensor_data *sw_data;

} app_data;

static app_data *g_ad = NULL;

static Eina_Bool __sw_to_value(void *data)
{
	app_data *ad = data;

	if (!ad) {
		_E("failed to get app_data");
		service_app_exit();
	}

	unsigned int delay_usec = 200000; // 20mS delay
	retv_if(!ad, -1);
	retv_if(!ad->sw_data, -1);

	resource_write_led(5, 0);
	usleep(delay_usec);
	resource_write_led(5, 1);
	usleep(delay_usec);

	resource_write_led(26, 0);
	usleep(delay_usec);
	resource_write_led(26, 1);
	usleep(delay_usec);

	return ECORE_CALLBACK_RENEW;
}

void gathering_stop(void *data)
{
	app_data *ad = data;

	ret_if(!ad);

	if (ad->getter_sw)
		ecore_timer_del(ad->getter_sw);
}

void gathering_start(void *data)
{
	app_data *ad = data;

	ret_if(!ad);

	gathering_stop(ad);

	ad->getter_sw = ecore_timer_add(SENSOR_GATHER_INTERVAL, __sw_to_value, ad);
	if (!ad->getter_sw)
		_E("Failed to add getter_sw");
}


static bool service_app_create(void *user_data)
{
	app_data *ad = (app_data *)user_data;

	ad->sw_data = sensor_data_new(SENSOR_DATA_TYPE_UINT);
	if (!ad->sw_data)
		return false;

	return true;
}

static void service_app_control(app_control_h app_control, void *user_data)
{

	gathering_start(user_data);

}

static void service_app_terminate(void *user_data)
{
	app_data *ad = (app_data *)user_data;

	resource_close_all();

	gathering_stop(ad);

	sensor_data_free(ad->sw_data);
	free(ad);
}

int main(int argc, char *argv[])
{
	app_data *ad = NULL;
	service_app_lifecycle_callback_s event_callback;

	ad = calloc(1, sizeof(app_data));
	retv_if(!ad, -1);

	g_ad = ad;

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	return service_app_main(argc, argv, &event_callback, ad);
}

