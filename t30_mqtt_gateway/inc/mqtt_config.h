/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__


//#define SERVER_ADDR		"broker.hivemq.com"
#define SERVER_ADDR		"3.65.154.195"

#define SERVER_PORT		1883
//#define SERVER_PORT		8000

#define APP_CONNECT_TIMEOUT_MS	2000
#define APP_SLEEP_MSECS		500

#define APP_CONNECT_TRIES	10

#define APP_MQTT_BUFFER_SIZE	128

#define MQTT_CLIENTID		        "vu_iot_group2_zephyr_esp32_"
#define MQTT_CLIENTID_STRLENGTH     27
#define MQTT_CLIENTID_TOTAL_LENGTH  50
#define MQTT_TOPIC  "tuwien_vu_iot/2021W/assignment3/group2/temperature"
#define MQTT_SUB_TOPIC  "tuwien_vu_iot/2021W/assignment3/group2/temperature_req"

#endif
