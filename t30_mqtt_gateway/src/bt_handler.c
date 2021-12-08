/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>

///////////////////////////////////////////////////////////////////////////////

static void start_scan(void);

uint8_t read_temp_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_read_params *params, 
			const void *data, uint16_t length);

void write_temp_range_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params);

///////////////////////////////////////////////////////////////////////////////

static struct bt_conn *default_conn;

static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;
static struct bt_gatt_read_params read_temp_params = 
{
	// configure read params
	.func = read_temp_func,
	.handle_count = 0,
	.by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE,
	.by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE,
	.by_uuid.uuid = BT_UUID_TEMPERATURE,
};

static unsigned int interval;

static struct bt_gatt_write_params write_temp_params = 
{
	.func = write_temp_range_func,
	.data = &interval,
	.length = sizeof(interval),
	.offset = 0,
};

static struct bt_conn *conn_device;

///////////////////////////////////////////////////////////////////////////////

uint8_t read_temp_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_read_params *params, 
			const void *data, uint16_t length)
{
	if(length >= sizeof(double))
	{
		const double *test= data;
		printk("[READ] data %g length %u\n", *test, length);
	}
	return BT_GATT_ITER_STOP;
}

void write_temp_range_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params)
{
	const unsigned int *value = params->data;
	printk("[WRITE] data %u , error code: %u\n", *value, -err);
}

static uint8_t discover_func(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		printk("Discover complete\n");
		(void)memset(params, 0, sizeof(*params));
		conn_device = conn; 
		return BT_GATT_ITER_STOP;
	}

	printk("[ATTRIBUTE] handle %u\n", attr->handle);

	if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_ESS)) {
		memcpy(&uuid, BT_UUID_TEMPERATURE, sizeof(uuid));
		discover_params.uuid = &uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		printk("Discovered service\n");

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}
	} 
	else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_TEMPERATURE)) {
		printk("Discovered Temperature\n");

		memcpy(&uuid, BT_UUID_ES_TRIGGER_SETTING, sizeof(uuid));
		discover_params.uuid = &uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}
	} 
	else if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_ES_TRIGGER_SETTING)) 
	{
		
		printk("Discovered Range\n");
		write_temp_params.handle = attr->handle +1;
		
		memcpy(&uuid, BT_UUID_GATT_CCC, sizeof(uuid));
		discover_params.uuid = &uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
				
		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}
		
	} 
	
	return BT_GATT_ITER_STOP;
}


static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	if (default_conn) {
		return;
	}

	/* We're only interested in connectable events */
	if (type != BT_GAP_ADV_TYPE_ADV_IND &&
	    type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		return;
	}

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	/* connect only to devices in close proximity */
	if (rssi < -70) {
		return;
	}

	if (bt_le_scan_stop()) {
		return;
	}

	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
				BT_LE_CONN_PARAM_DEFAULT, &default_conn);
	if (err) {
		printk("Create conn to %s failed (%u)\n", addr_str, err);
		start_scan();
	}
}

static void start_scan(void)
{
	int err;

	/* This demo doesn't require active scan */
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printk("Failed to connect to %s (%u)\n", addr, err);

		bt_conn_unref(default_conn);
		default_conn = NULL;

		start_scan();
		return;
	}

	if (conn != default_conn) {
		return;
	}

	printk("Connected: %s\n", addr);

	memcpy(&uuid, BT_UUID_ESS, sizeof(uuid));
	discover_params.uuid = &uuid.uuid;
	discover_params.func = discover_func;
	discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
	discover_params.type = BT_GATT_DISCOVER_PRIMARY;

	err = bt_gatt_discover(default_conn, &discover_params);
	if (err) {
		printk("Discover failed(err %d)\n", err);
		return;
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != default_conn) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	bt_conn_unref(default_conn);
	default_conn = NULL;

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

int bt_handler_init(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	start_scan();
	
	//return 0;
	
	while(conn_device == NULL)
	{
		k_msleep(1000);
	}
	
	printk("Device connected\n");
	
	// cycle through update intervals and read values
	while(true)
	{
	
		for(uint8_t i=1; i<= 10 ; i++)
		{
			interval = i*1000;
			bt_gatt_write(conn_device, &write_temp_params);

			for(uint8_t j=0; j< 10; j++)
			{
				k_msleep(1000);
				bt_gatt_read(conn_device, &read_temp_params);
			}
		}

	}
	return 0;
}