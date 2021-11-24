#include <zephyr.h>
#include <drivers/sensor.h>
#include <device.h>
#include <devicetree.h>

// include app files
#include "sensors.h"

#define TEMPERATURE_LOWER_THRESHOLD 20.0
#define TEMPERATURE_UPPER_THRESHOLD 25.0

void main() {
    printk("Starting main.\n");

    sensors_temperature_init(TEMPERATURE_LOWER_THRESHOLD, TEMPERATURE_UPPER_THRESHOLD);

    // counting loop, to show some progress
    int cnt =0;
    while(true) {
        k_msleep(500);
        printk("Iteration %u\n", ++cnt);
        k_msleep(500);
    }

  return;
}