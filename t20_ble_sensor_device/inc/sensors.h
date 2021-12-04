#ifndef APP_SENSORS_H
#define APP_SENSORS_H

struct temp_range_t
{
    double lower;
    double upper;
};

// init temperature sensor
void sensors_temperature_init(int lower, int upper);

// set trigger threshold
void sensors_temperature_set_notification_range(struct temp_range_t *range);
void sensors_temperature_set_window(int lower, int upper);
void sensors_temperature_set_lower_threshold(int lower);
void sensors_temperature_set_upper_threshold(int upper);

// get current temperatur
double sensors_get_current_temperature(void);

#endif