/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__


#define SERVER_ADDR		"broker.hivemq.com"

#ifdef CONFIG_MQTT_LIB_WEBSOCKET
#define SERVER_PORT		8000
#else
#define SERVER_PORT		1883
#endif

#define APP_CONNECT_TIMEOUT_MS	2000
#define APP_SLEEP_MSECS		500

#define APP_CONNECT_TRIES	10

#define APP_MQTT_BUFFER_SIZE	128

#define MQTT_CLIENTID		"vu_iot_group2_zephyr_esp32"
#define MQTT_TOPIC  "tuwien_vu_io/group2/temperature"
#endif
