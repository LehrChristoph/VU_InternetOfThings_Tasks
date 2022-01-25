/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <zephyr.h>

#include "handlers.h"
#include "kernel.h"

void main(void)
{
	wifi_handler_connect(true);
	bt_handler_init();
	bt_handler_set_sampling_interval(1000);
	while(mqtt_handler_connect())
	{
		k_msleep(1000);
	}

    bool send = false;
    mqtt_set_send(&send);

    unsigned int sampling_interval=10000;
    uint64_t last_sample_sent = k_uptime_get();
	while(true)
	{
        // bt_get_temp() returns the last correct reading
        if (send || k_uptime_get() - last_sample_sent >= sampling_interval)
        {
            if(bt_handler_fetch_data())
            {
                break;
            }
            if(mqtt_handler_publish(bt_get_temp()))
            {
                break;
            }
            send = false;
            last_sample_sent = k_uptime_get();
        }
        else
        {
            if (mqtt_handler_keep_alive())
            {
                break;
            }
        }
		/* k_msleep(10000); */

        uint64_t start = k_uptime_get();
        while (k_uptime_get() - start < 1000 && !send)
        {
        }
	}

	mqtt_handler_disconnect();

}
