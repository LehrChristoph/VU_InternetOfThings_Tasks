#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/gap.h>
#include <bluetooth/gatt.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>
#include <string.h>

// include app files
#include "sensors.h"

#define TEMPERATURE_LOWER_THRESHOLD 20.0
#define TEMPERATURE_UPPER_THRESHOLD 25.0

///////////////////////////////////////////////////////////////////////////////

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LENGTH (sizeof(DEVICE_NAME) - 1)

// example user data
#define DATA_LENGTH 20
static uint8_t data[DATA_LENGTH + 1] = { 'H', 'a','l', 'l', 'o'};

// UUID of the service (random)
static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xb47244dc, 0x4d40, 0x11ec, 0x81d3, 0x0242ac130003));

// adverticing data (its name)
static const struct bt_data advertisement[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LENGTH),
};

// characteristic 1: READ
// UUID of the first characteristic (random)
static struct bt_uuid_128 characteristic1_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x341a0ba2, 0x4d41, 0x11ec, 0x81d3, 0x0242ac130003));

ssize_t characteristic1_read_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset) {

    const char *value = attr->user_data;

    printk("read value: %s\n", value);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

// characteristic 2: WRITE
// UUID of the second characteristic (random)
static struct bt_uuid_128 characteristic2_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x0d9575d0, 0xcccf, 0x4762, 0xb0e8, 0x3aa50ffb0db0));

ssize_t characteristic2_write_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags) {

    uint8_t *value = attr->user_data;

    if (offset + len > DATA_LENGTH) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);

    value[offset + len] = 0;
                 
    printk("write value: %s\n", (char *)buf);

    return len;
}

// definition of service
BT_GATT_SERVICE_DEFINE(
    srv,
    BT_GATT_PRIMARY_SERVICE(&service_uuid),

    // (one characteristic could do both, split just for exercise)
    // read characteristic
    BT_GATT_CHARACTERISTIC(
        &characteristic1_uuid.uuid,
        BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        characteristic1_read_callback,
        NULL,
        data
    ),

    // write characteristic
    BT_GATT_CHARACTERISTIC(
        &characteristic2_uuid.uuid,
        BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_WRITE,
        NULL,
        characteristic2_write_callback,
        data
    ),
);


///////////////////////////////////////////////////////////////////////////////


void main() {
    printk("Starting main.\n");

    // sensors_temperature_init(TEMPERATURE_LOWER_THRESHOLD, TEMPERATURE_UPPER_THRESHOLD);

    // // counting loop, to show some progress
    // int cnt =0;
    // while(true) {
    //     k_msleep(500);
    //     printk("Iteration %u\n", ++cnt);
    //     k_msleep(500);
    // }

    int err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, advertisement, ARRAY_SIZE(advertisement), NULL, 0);

    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    } else {
        printk("Started advertising.\n");
    }
}
