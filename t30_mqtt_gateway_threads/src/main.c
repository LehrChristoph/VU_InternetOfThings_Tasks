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


#define MY_STACK_SIZE 128
#define MY_PRIORITY 5

K_THREAD_STACK_DEFINE(my_stack_area, 256);
//K_THREAD_STACK_DEFINE(my_stack_area_2, 128);

void bt_fetcher(void)
{
	printk("BT Thread started\n");
	while (1)
	{

		printk("BT Thread polling\n");
		/* code */
		if(bt_handler_fetch_data())
		{
			break;
		}
		if(mqtt_handler_publish(bt_get_temp()))
		{
			//break;
			printk("MQTT Error\n");
		}
		k_msleep(10000);
	}
}

void mqtt_keep_alive_thread(void)
{
	printk("Alive thread started\n");
	while (1)
	{
		printk("keep alive\n");
		int ret = mqtt_keep_alive();
		if(ret < 0)
		{
			printk("Keep alive error\n");
			return;
		}
		printk("Alive Interval: %d\n", ret);
		k_msleep(ret >>1);
	}
}

void main(void)
{
	printk("Starting System\n");

	wifi_handler_connect(true);
	bt_handler_init();
	bt_handler_set_sampling_interval(1000);
	while(mqtt_handler_connect())
	{
		k_msleep(1000);
	}
	
	struct k_thread my_thread_data, my_thread_data_2;

	k_tid_t my_tid = k_thread_create(&my_thread_data, my_stack_area,
									K_THREAD_STACK_SIZEOF(my_stack_area),
									bt_fetcher,
									NULL, NULL, NULL,
									5, 0, K_NO_WAIT);
	/*
	k_tid_t my_tid_2 = k_thread_create(&my_thread_data_2, my_stack_area_2,
									K_THREAD_STACK_SIZEOF(my_stack_area_2),
									mqtt_keep_alive_thread,
									NULL, NULL, NULL,
									4, 0, K_NO_WAIT);
	*/
    bool send = false;
    mqtt_set_send(&send);

	while(true)
	{
		/*
		if(bt_handler_fetch_data())
		{
			break;
		}*/
        // bt_get_temp() returns the last correct reading
		/*
		if(mqtt_handler_publish(bt_get_temp()))
		{
			//break;
			printk("MQTT Error\n");
		}
		*/
		/* k_msleep(10000); */
        /*
		uint64_t start = k_uptime_get();
        while (k_uptime_get() - start < 5000 && !send)
        {
        }
		*/
		
		int ret = mqtt_keep_alive();
		if(ret < 0)
		{
			printk("Keep alive error\n");
			return;
		}
		printk("Alive Interval: %d\n", ret);
		k_msleep(ret >>1);
		
        send = false;
	}

	mqtt_handler_disconnect();

}
