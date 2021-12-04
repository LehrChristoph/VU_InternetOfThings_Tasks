#include <zephyr.h>

// include app files
#include "sensors.h"
#include "bt_sensor_service.h"

#define TEMPERATURE_LOWER_THRESHOLD 20.0
#define TEMPERATURE_UPPER_THRESHOLD 25.0


void main() {
    printk("Starting main.\n");

    sensors_temperature_init(TEMPERATURE_LOWER_THRESHOLD, TEMPERATURE_UPPER_THRESHOLD);
    bt_service_init();

    // counting loop, to show some progress
    int cnt =0;
    while(true) {
        k_msleep(1000);
        sensors_get_current_temperature();    
    }
}
