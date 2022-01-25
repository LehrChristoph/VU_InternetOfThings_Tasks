#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/gap.h>
#include <bluetooth/gatt.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>
#include <string.h>

#include "sensors.h"
#include "rtc.h"

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

typedef struct temp_value
{
    double temperature;
    uint64_t timestamp;
}temp_value_t;

static temp_value_t current_temp;

static unsigned int update_period;

static struct bt_conn *conn_device;
static const struct bt_gatt_attr *conn_attr;

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
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

    // (one characteristic could do both, split just for exercise)
    // read characteristic
    BT_GATT_CHARACTERISTIC(
        BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ ,
        BT_GATT_PERM_READ,
        char_temp_read_callback,
        NULL,
        NULL
    ),

    // write characteristic
    BT_GATT_CHARACTERISTIC(
        BT_UUID_ES_TRIGGER_SETTING,
        BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_WRITE,
        NULL,
        char_range_write_callback,
        &update_period
    ),
);

///////////////////////////////////////////////////////////////////////////////

ssize_t char_temp_read_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset) {
    
    current_temp.temperature = sensors_get_current_temperature();
    current_temp.timestamp = rtc_get_timestamp();
    const temp_value_t *value = &current_temp;

    printk("read value %lf at %llu\n", current_temp.temperature, current_temp.timestamp);

    conn_device = conn;
    conn_attr = attr;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(temp_value_t));
}

ssize_t char_range_write_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags) {

    unsigned int *value = attr->user_data;

    if (offset + len > sizeof(update_period)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);

    printk("write value: %u\n", *value);
                 
    return len;
}

unsigned long bt_service_get_update_interval(void)
{
    return update_period;
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

    update_period = 1000;

    return 0;
}