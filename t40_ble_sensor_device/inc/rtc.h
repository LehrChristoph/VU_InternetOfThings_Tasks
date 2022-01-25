
#ifndef APP_RTC_INCLUDE_H
#define APP_RTC_INCLUDE_H

#include <zephyr.h>

void rtc_init(void);

uint64_t rtc_get_timestamp(void);

#endif