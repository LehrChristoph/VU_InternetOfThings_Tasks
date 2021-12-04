#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/gap.h>
#include <bluetooth/gatt.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>
#include <string.h>

#include "sensors.h"

///////////////////////////////////////////////////////////////////////////////

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LENGTH (sizeof(DEVICE_NAME) - 1)                                                                                             

///////////////////////////////////////////////////////////////////////////////

ssize_t char_temp_read_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset);

ssize_t char_range_write_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags);

///////////////////////////////////////////////////////////////////////////////


static double current_temp; 
static struct temp_range_t range;

static struct bt_conn *conn_device;
static const struct bt_gatt_attr *conn_attr;

// UUID of the service (random)
static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xb47244dc, 0x4d40, 0x11ec, 0x81d3, 0x0242ac130003));

// advertising data (its name)
static const struct bt_data advertisement[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    //BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LENGTH),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_TEMPERATURE_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_VALID_RANGE_VAL)
    )
};

// definition of service
BT_GATT_SERVICE_DEFINE(
    srv,
    BT_GATT_PRIMARY_SERVICE(&service_uuid),

    // (one characteristic could do both, split just for exercise)
    // read characteristic
    BT_GATT_CHARACTERISTIC(
        BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        char_temp_read_callback,
        NULL,
        NULL
    ),

    // write characteristic
    BT_GATT_CHARACTERISTIC(
        BT_UUID_VALID_RANGE,
        BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_WRITE,
        NULL,
        char_range_write_callback,
        &range
    ),
);

///////////////////////////////////////////////////////////////////////////////

ssize_t char_temp_read_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset) {

    current_temp = sensors_get_current_temperature();
    const double *value = &current_temp;

    printk("read value: %lf\n", current_temp);

    conn_device = conn;
    conn_attr = attr;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(double));
}

int bt_service_char_temp_notify(void)
{
    
    printk("notify value: %lf\n", current_temp);
  
    return bt_gatt_notify(conn_device, conn_attr, &current_temp, sizeof(current_temp));
}

ssize_t char_range_write_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags) {

    struct temp_range_t *value = attr->user_data;

    if (offset + len > sizeof(struct temp_range_t)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);
    
    sensors_temperature_set_notification_range(value);

    printk("write value: %lf - %lf\n", value->lower, value->upper);
                 
    return len;
}

int bt_service_init(void)
{
    int err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return err;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, advertisement, ARRAY_SIZE(advertisement), NULL, 0);

    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return err;
    } else {
        printk("Started advertising.\n");
    }

    return 0;
}