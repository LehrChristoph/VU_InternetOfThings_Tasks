#include <zephyr.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>

#include "sensors.h"

// Important: Lowercase, convert @ to _ and - to _
#define SENSOR_TEMPERATURE DT_PATH(soc, i2c_40003000, hts221_5f)
#define SENSOR_TEMPERATURE_LABEL DT_NODELABEL(hts221)

static struct sensor_value lower_threshold;
static struct sensor_value upper_threshold;

const struct device* temp_sensor_device;

static void sensors_temperature_trigger_handler(const struct device *dev,
			                struct sensor_trigger *trig);

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

    // configure thesholds
    sensors_temperature_set_window(lower, upper);

    // provide upper and lower threshold
    struct sensor_trigger trigger;
    trigger.type = SENSOR_TRIG_DATA_READY;
    trigger.chan = SENSOR_CHAN_AMBIENT_TEMP;
    if(sensor_trigger_set(temp_sensor_device, &trigger, sensors_temperature_trigger_handler))
    {
        printk("\nError: Could not set trigger handler for Device \"%s\"\n",
            temp_sensor_device->name);
        return;
    }
}

void sensors_temperature_set_window(int lower, int upper)
{
    // convert lower threshold to celcius and micro celsius
    sensor_value_from_double(&lower_threshold, lower);
    sensor_value_from_double(&upper_threshold, upper);
}

void sensors_temperature_set_lower_threshold(int lower, int upper)
{
    // convert lower threshold to celcius and micro celsius
    sensor_value_from_double(&lower_threshold, lower);
}

void sensors_temperature_set_upper_threshold(int lower, int upper)
{
    // convert lower threshold to celcius and micro celsius
    sensor_value_from_double(&upper_threshold, upper);
}

static void sensors_temperature_trigger_handler(const struct device *dev,
			                struct sensor_trigger *trig)
{
    struct sensor_value temp;
	static size_t cnt_upper, cnt_lower;
	int rc;

	// fetch data from sensor
    rc = sensor_sample_fetch(dev);
	if (rc != 0) {
		printk("sensor_sample_fetch error: %d\n", rc);
		return;
	}
    // retreive from zephyr
	rc = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	if (rc != 0) {
		printk("sensor_channel_get error: %d\n", rc);
		return;
	}

    // micro celsius are ignored when configuring threshhold,
    // therefore there ignored here to
    if(temp.val1 < lower_threshold.val1 || 
        (temp.val1 == lower_threshold.val1 && temp.val2 < lower_threshold.val2))
    {
        printk("Lower threshold exceeded %u, temp %g deg C\n", ++cnt_lower,
	       sensor_value_to_double(&temp));
    }
    else if(temp.val1 > upper_threshold.val1 ||
            (temp.val1 == upper_threshold.val1 && temp.val2 > upper_threshold.val2))
    {
	    printk("Upper threshold exceeded %u, temp %g deg C\n", ++cnt_upper,
	       sensor_value_to_double(&temp));
    }
}