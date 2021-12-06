#ifndef APP_SENSORS_H
#define APP_SENSORS_H

struct temp_range_t
{
    double lower;
    double upper;
};

// init temperature sensor
void sensors_temperature_init(int lower, int upper);

// get current temperatur
double sensors_get_current_temperature(void);

double sensors_fetch_temperature_data(void);

#endif