#include <zephyr.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>

// Important: Lowercase, convert @ to _ and - to _
#define PRESSURE_SENSOR DT_PATH(soc, i2c_40003000, lps22hb_press_5c)
#define PRESSURE_SENSOR_LABEL DT_NODELABEL(lps22hb_press)

void main() {
    printk("Starting main.\n");

#if !DT_NODE_EXISTS(PRESSURE_SENSOR_LABEL)
    #error "The node does not exist, something is wrong!"
#endif

#if DT_NODE_HAS_STATUS(PRESSURE_SENSOR, okay)
    const struct device* sensor_device = device_get_binding(DT_LABEL(PRESSURE_SENSOR));
#else
    #error "Node is disabled"
#endif

    if (sensor_device == NULL) {
        /* No such node, or the node does not have status "okay". */
        printk("\nError: no device found.\n");
        return;
    }

    if (!device_is_ready(sensor_device)) {
        printk("\nError: Device \"%s\" is not ready; "
                "check the driver initialization logs for errors.\n",
                sensor_device->name);
        return;
    }

    printk("Found device \"%s\", getting sensor data\n", sensor_device->name);

    struct sensor_value pressure_value;
    int err;
    while(true) {
    err = sensor_sample_fetch_chan(sensor_device, SENSOR_CHAN_PRESS);
    if(err){
        printk("Error when sampling sensor (err: %d)", err);
    }

    err = sensor_channel_get(sensor_device, SENSOR_CHAN_PRESS,
                                &pressure_value);
    if(err){
        printk("Error obtaining sensor value (err: %d)", err);
    }

    printk("Pressure: %f \n",  sensor_value_to_double(&pressure_value));
    k_msleep(1000);
    }

    return;
}