/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_mqtt_publisher_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <net/socket.h>
#include <net/mqtt.h>
#include <random/rand32.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "handlers.h"
#include "mqtt_config.h"
///////////////////////////////////////////////////////////////////////////////

#define APP_BMEM
#define APP_DMEM

/* Buffers for MQTT client. */
static APP_BMEM uint8_t rx_buffer[APP_MQTT_BUFFER_SIZE];
static APP_BMEM uint8_t tx_buffer[APP_MQTT_BUFFER_SIZE];

/* The mqtt client struct */
static APP_BMEM struct mqtt_client client_ctx;
static char clientID[MQTT_CLIENTID_TOTAL_LENGTH]; 

#if defined(CONFIG_MQTT_LIB_WEBSOCKET)
#define MQTT_LIB_WEBSOCKET_RECV_BUF_LEN 1280

/* Websocket needs temporary buffer to store partial packets */
static APP_BMEM uint8_t temp_ws_rx_buf[MQTT_LIB_WEBSOCKET_RECV_BUF_LEN];
#endif

/* MQTT Broker details. */
static APP_BMEM struct sockaddr_storage broker;

static APP_BMEM struct zsock_pollfd fds[1];
static APP_BMEM int nfds;

static APP_BMEM bool connected;

///////////////////////////////////////////////////////////////////////////////

#define RC_STR(rc) ((rc) == 0 ? "OK" : "ERROR")

#define PRINT_RESULT(func, rc) \
	printk("[MQTT] func: %s: %d <%s>\n", (func), rc, RC_STR(rc))

#define SUCCESS_OR_EXIT(rc) { if (rc != 0) { return 1; } }

///////////////////////////////////////////////////////////////////////////////

bool *send_ref;

void mqtt_set_send(bool *send)
{
    send_ref = send;
}

static void prepare_fds(struct mqtt_client *client)
{
	if (client->transport.type == MQTT_TRANSPORT_NON_SECURE) {
		fds[0].fd = client->transport.tcp.sock;
	}

	fds[0].events = ZSOCK_POLLIN;
	nfds = 1;
}

static void clear_fds(void)
{
	nfds = 0;
}

static int wait(int timeout)
{
	int ret = 0;

	if (nfds > 0) {
		ret = zsock_poll(fds, nfds, timeout);
		if (ret < 0) {
			printk("[MQTT] ERROR:poll error: %d\n", errno);
		}
	}

	return ret;
}

void mqtt_evt_handler(struct mqtt_client *const client,
		      const struct mqtt_evt *evt)
{
	int err;

	switch (evt->type) {
	case MQTT_EVT_CONNACK:
		if (evt->result != 0) {
			printk("[MQTT] ERROR: connect failed %d\n", evt->result);
			break;
		}

		connected = true;
		printk("[MQTT]:  client connected!\n");

		break;

	case MQTT_EVT_DISCONNECT:
		printk("[MQTT]:  client disconnected %d\n", evt->result);

		connected = false;
		clear_fds();

		break;

	case MQTT_EVT_PUBACK:
		if (evt->result != 0) {
			printk("[MQTT] ERROR: PUBACK error %d\n", evt->result);
			break;
		}

		printk("[MQTT]: PUBACK packet id: %u\n", evt->param.puback.message_id);

		break;

	case MQTT_EVT_PUBREC:
		if (evt->result != 0) {
			printk("[MQTT] ERROR: PUBREC error %d\n", evt->result);
			break;
		}

		printk("[MQTT]: PUBREC packet id: %u\n", evt->param.pubrec.message_id);

		const struct mqtt_pubrel_param rel_param = {
			.message_id = evt->param.pubrec.message_id
		};

		err = mqtt_publish_qos2_release(client, &rel_param);
		if (err != 0) {
			printk("[MQTT] ERROR:Failed to send MQTT PUBREL: %d\n", err);
		}

		break;

	case MQTT_EVT_PUBCOMP:
		if (evt->result != 0) {
			printk("[MQTT] ERROR: PUBCOMP error %d\n", evt->result);
			break;
		}

		printk("[MQTT]: PUBCOMP packet id: %u\n",
			evt->param.pubcomp.message_id);

		break;

	case MQTT_EVT_PINGRESP:
		printk("[MQTT]: PINGRESP packet\n");
		break;

    case MQTT_EVT_PUBLISH:
        if (evt->result != 0) {
            printk("[MQTT] Error: PUBLISH error %d\n", evt->result);
        }

        char buffer[1024];
        int rc = mqtt_read_publish_payload(client, buffer, sizeof(buffer));
        if (rc < 0)
        {
            printk("[MQTT] Error: failed to read published payload: %d\n", rc);
        }

        /* printk("[MQTT] Reveived: %s\nPublish Temperature ...\n", buffer); */
        printk("[MQTT] Reveived: %s\n", buffer);

        // publish temperature if we receive something on the sub-topic
        // TODO: Publish TEMP
        *send_ref = true;

        break;

	default:
		break;
	}
}

static char *get_mqtt_payload(double temp)
{
	// static APP_DMEM char payload[] = "DOORS:OPEN_QoSx";
	static APP_DMEM char payload[50];

    if (temp != 0)
    {
        snprintf(payload, 50, "Temperature: %f", temp);
    }
    else {
        snprintf(payload, 50, "no temperature value availible");
    }

	// payload[strlen(payload) - 1] = '0' + qos;

	return payload;
}

static char *get_mqtt_topic(void)
{
	return MQTT_TOPIC;
}

static char *get_mqtt_sub_topic(void)
{
	return MQTT_SUB_TOPIC;
}

static int sub_topic(struct mqtt_client *client, enum mqtt_qos qos)
{
    struct mqtt_topic topic;
    topic.qos = qos;
    topic.topic.utf8 = (uint8_t *)get_mqtt_sub_topic();
    topic.topic.size = strlen(topic.topic.utf8);

    struct mqtt_subscription_list subscription_list = {
        .list = &topic,
        .list_count = 1,
        .message_id = 1 // ???
    };

    return mqtt_subscribe(client, &subscription_list);
}

static int publish(struct mqtt_client *client, enum mqtt_qos qos, double temp)
{
	struct mqtt_publish_param param;

	param.message.topic.qos = qos;
	param.message.topic.topic.utf8 = (uint8_t *)get_mqtt_topic();
	param.message.topic.topic.size =
			strlen(param.message.topic.topic.utf8);
	param.message.payload.data = get_mqtt_payload(temp);
	param.message.payload.len =
			strlen(param.message.payload.data);
	param.message_id = sys_rand32_get();
	param.dup_flag = 0U;
	param.retain_flag = 0U;

	return mqtt_publish(client, &param);
}



static void broker_init(void)
{
	struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;

	broker4->sin_family = AF_INET;
	broker4->sin_port = htons(SERVER_PORT);
	zsock_inet_pton(AF_INET, SERVER_ADDR, &broker4->sin_addr);
}

static void rand_string(char *str, int start, int size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
    if (size) {
        --size;
        for (int n = start; n < size; n++) {
            int key =  sys_rand32_get() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return ;
}

static void client_init(struct mqtt_client *client)
{
	mqtt_client_init(client);

	broker_init();
	memcpy(clientID, MQTT_CLIENTID, MQTT_CLIENTID_STRLENGTH);
	rand_string(clientID, MQTT_CLIENTID_STRLENGTH, MQTT_CLIENTID_TOTAL_LENGTH-1);
	printk("ClientID: %s\n", clientID);

	/* MQTT client configuration */
	client->broker = &broker;
	client->evt_cb = mqtt_evt_handler;
	client->client_id.utf8 = (uint8_t *)clientID;
	client->client_id.size = strlen(clientID);
	client->password = NULL;
	client->user_name = NULL;
	client->protocol_version = MQTT_VERSION_3_1_1;


	/* MQTT buffers configuration */
	client->rx_buf = rx_buffer;
	client->rx_buf_size = sizeof(rx_buffer);
	client->tx_buf = tx_buffer;
	client->tx_buf_size = sizeof(tx_buffer);

	/* MQTT transport configuration */

#if defined(CONFIG_MQTT_LIB_WEBSOCKET)
	client->transport.type = MQTT_TRANSPORT_NON_SECURE_WEBSOCKET;
	client->transport.websocket.config.host = SERVER_ADDR;
	client->transport.websocket.config.url = "/mqtt";
	client->transport.websocket.config.tmp_buf = temp_ws_rx_buf;
	client->transport.websocket.config.tmp_buf_len =
						sizeof(temp_ws_rx_buf);
	client->transport.websocket.timeout = 5 * MSEC_PER_SEC;
#else
	client->transport.type = MQTT_TRANSPORT_NON_SECURE;
#endif

}

/* In this routine we block until the connected variable is 1 */
static int try_to_connect(struct mqtt_client *client)
{
	int rc, i = 0;

	while (i++ < APP_CONNECT_TRIES && !connected) {
		printk("[MQTT]: Client init\n");
		client_init(client);

		printk("[MQTT]: connecting\n");
		rc = mqtt_connect(client);

		if (rc != 0) {
			PRINT_RESULT("mqtt_connect", rc);
			k_sleep(K_MSEC(APP_SLEEP_MSECS));
			continue;
		}

		prepare_fds(client);

		if (wait(APP_CONNECT_TIMEOUT_MS)) {
			mqtt_input(client);
		}

		if (!connected) {
			mqtt_abort(client);
		}

        rc = sub_topic(client, MQTT_QOS_0_AT_MOST_ONCE);

        if (rc != 0) {
            PRINT_RESULT("sub_topic", rc);
			k_sleep(K_MSEC(APP_SLEEP_MSECS));
			continue;
        }
        else {
            printk("[MQTT] Subscribed to topic '%s' with QOS=%d\n",
                get_mqtt_sub_topic(), MQTT_QOS_0_AT_MOST_ONCE);
        }
	}

	if (connected) {
		return 0;
	}

	return -EINVAL;
}

static int process_mqtt_and_sleep(struct mqtt_client *client, int timeout)
{
	int64_t remaining = timeout;
	int64_t start_time = k_uptime_get();
	int rc;

	while (remaining > 0 && connected) {
		if (wait(remaining)) {
			rc = mqtt_input(client);
			if (rc != 0) {
				PRINT_RESULT("mqtt_input", rc);
				return rc;
			}
		}

		rc = mqtt_live(client);
		if (rc != 0 && rc != -EAGAIN) {
			PRINT_RESULT("mqtt_live", rc);
			return rc;
		} else if (rc == 0) {
			rc = mqtt_input(client);
			if (rc != 0) {
				PRINT_RESULT("mqtt_input", rc);
				return rc;
			}
		}

		remaining = timeout + start_time - k_uptime_get();
	}

	return 0;
}


int mqtt_handler_connect(void)
{
	int rc;

	printk("[MQTT]: attempting to connect ...\n");
	rc = try_to_connect(&client_ctx);
	PRINT_RESULT("try_to_connect", rc);
	return rc;
}

int mqtt_handler_publish(double temp)
{
	int rc;

	rc = mqtt_ping(&client_ctx);
	PRINT_RESULT("mqtt_ping", rc);
	if (rc != 0)
	{
		return rc;
	}

	rc = process_mqtt_and_sleep(&client_ctx, APP_SLEEP_MSECS);
	if (rc != 0)
	{
		return rc;
	}

	rc = publish(&client_ctx, MQTT_QOS_0_AT_MOST_ONCE, temp);
	PRINT_RESULT("publish", rc);
	if (rc != 0)
	{
		return rc;
	}

	rc = process_mqtt_and_sleep(&client_ctx, APP_SLEEP_MSECS);

	return rc;
}

int mqtt_handler_keep_alive()
{
    int rc;

	rc = mqtt_ping(&client_ctx);
	PRINT_RESULT("mqtt_ping", rc);
	if (rc != 0)
	{
		return rc;
	}

	rc = process_mqtt_and_sleep(&client_ctx, APP_SLEEP_MSECS);
	return rc;
}

int mqtt_handler_disconnect()
{
	int rc = mqtt_disconnect(&client_ctx);
	PRINT_RESULT("mqtt_disconnect", rc);

	printk("[MQTT]: Bye!\n");

	return rc;
}
