#include <zephyr.h>

// include app files
#include "sensors.h"
#include "bt_sensor_service.h"
#include "rtc.h"

#define TEMPERATURE_LOWER_THRESHOLD 20.0
#define TEMPERATURE_UPPER_THRESHOLD 25.0


void main() {
    printk("Starting main.\n");

    sensors_temperature_init(TEMPERATURE_LOWER_THRESHOLD, TEMPERATURE_UPPER_THRESHOLD);
    bt_service_init();
    rtc_init();

    // counting loop, to show some progress
    unsigned int interval =0;
    
    while(true) {
        interval = bt_service_get_update_interval();
        printk("waiting for %u\n", interval);
        k_msleep(interval);
        sensors_fetch_temperature_data();    

    }
}
