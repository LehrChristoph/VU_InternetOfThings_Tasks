#include <zephyr.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>

#include "sensors.h"
#include "bt_sensor_service.h"

// Important: Lowercase, convert @ to _ and - to _
#define SENSOR_TEMPERATURE DT_PATH(soc, i2c_40003000, hts221_5f)
#define SENSOR_TEMPERATURE_LABEL DT_NODELABEL(hts221)

volatile double current_temp; 

const struct device* temp_sensor_device;

void sensors_temperature_init(int lower, int upper)
{
#if !DT_NODE_EXISTS(SENSOR_TEMPERATURE_LABEL)
    #error "The node does not exist, something is wrong!"
#endif

#if DT_NODE_HAS_STATUS(SENSOR_TEMPERATURE, okay)
    temp_sensor_device = device_get_binding(DT_LABEL(SENSOR_TEMPERATURE));
#else
    #error "Node is disabled"
#endif
    
    if (temp_sensor_device == NULL) {
        /* No such node, or the node does not have status "okay". */
        printk("\nError: no device found.\n");
        return;
    }

    // check if device is fully iniatilized
    if (!device_is_ready(temp_sensor_device)) {
        printk("\nError: Device \"%s\" is not ready; "
            "check the driver initialization logs for errors.\n",
            temp_sensor_device->name);
        return;
    }
}

double sensors_get_current_temperature(void)
{
    return current_temp;
}

double sensors_fetch_temperature_data(void)
{
    struct sensor_value temp;
	int rc;

	// fetch data from sensor
    rc = sensor_sample_fetch(temp_sensor_device);
	if (rc != 0) {
		printk("sensor_sample_fetch error: %d\n", rc);
		return rc ;
	}
    // retreive from zephyr
	rc = sensor_channel_get(temp_sensor_device, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	if (rc != 0) {
		printk("sensor_channel_get error: %d\n", rc);
		return rc;
	}

    current_temp = sensor_value_to_double(&temp);
    return current_temp;
}


