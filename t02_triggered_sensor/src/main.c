#include <zephyr.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>

// Important: Lowercase, convert @ to _ and - to _
#define SENSOR_TEMPERATURE DT_PATH(soc, i2c_40003000, hts221_5f)
#define SENSOR_TEMPERATURE_LABEL DT_NODELABEL(hts221)

#define TEMPERATURE_LOWER_THRESHOLD 20
#define TEMPERATURE_UPPER_THRESHOLD 25

static int set_window(const struct device *dev, int lower, int upper)
{
    // converte lower threshold to celcius and micro celsius
	struct sensor_value val = {
		.val1 = lower,
		.val2 = 0,
	};
	int rc = sensor_attr_set(dev, SENSOR_CHAN_AMBIENT_TEMP,
				                SENSOR_ATTR_LOWER_THRESH, &val);
	if (rc == 0) {
        // converte upper threshold to celcius and micro celsius
		val.val1 = upper,
		rc = sensor_attr_set(dev, SENSOR_CHAN_AMBIENT_TEMP,
				                SENSOR_ATTR_UPPER_THRESH, &val);
	}
    else{
        printk("Error val: %d", rc );
    }

	return rc;
}


static void trigger_handler(const struct device *dev,
			                struct sensor_trigger *trig)
{
    struct sensor_value temp;
	static size_t cnt;
	int rc;

	++cnt;
	rc = sensor_sample_fetch(dev);
	if (rc != 0) {
		printk("sensor_sample_fetch error: %d\n", rc);
		return;
	}
	rc = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	if (rc != 0) {
		printk("sensor_channel_get error: %d\n", rc);
		return;
	}

	printk("trigger fired %u, temp %g deg C\n", cnt,
	       sensor_value_to_double(&temp));
	
    //set_window(dev, TEMPERATURE_LOWER_THRESHOLD, TEMPERATURE_UPPER_THRESHOLD);
}

void main() {
    printk("Starting main.\n");

#if !DT_NODE_EXISTS(SENSOR_TEMPERATURE_LABEL)
    #error "The node does not exist, something is wrong!"
#endif

#if DT_NODE_HAS_STATUS(SENSOR_TEMPERATURE, okay)
    const struct device* sensor_device = device_get_binding(DT_LABEL(SENSOR_TEMPERATURE));
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


    if(set_window(sensor_device, TEMPERATURE_LOWER_THRESHOLD, TEMPERATURE_UPPER_THRESHOLD))
    {
        printk("\nError: Could not set window for Device \"%s\"\n",
            sensor_device->name);
        return;
    }

    // provide upper and lower threshold
    struct sensor_trigger trigger;
    trigger.type = SENSOR_TRIG_THRESHOLD;
    trigger.chan = SENSOR_CHAN_AMBIENT_TEMP;
    if(sensor_trigger_set(sensor_device, &trigger, trigger_handler))
    {
        printk("\nError: Could not set trigger handler for Device \"%s\"\n",
            sensor_device->name);
        return;
    }

    printk("Found device \"%s\", getting sensor data\n", sensor_device->name);

    struct sensor_value temp_value;
    int err;
    while(true) {
    err = sensor_sample_fetch_chan(sensor_device, SENSOR_CHAN_AMBIENT_TEMP);
    if(err){
        printk("Error when sampling sensor (err: %d)", err);
    }

    err = sensor_channel_get(sensor_device, SENSOR_CHAN_AMBIENT_TEMP,
                                &temp_value);
    if(err){
        printk("Error obtaining sensor value (err: %d)", err);
    }

    printk("Temperature: %f \n",  sensor_value_to_double(&temp_value));
    k_msleep(1000);
    }

  return;
}