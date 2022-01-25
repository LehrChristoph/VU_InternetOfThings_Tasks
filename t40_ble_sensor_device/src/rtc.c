
#include <device.h>
#include <devicetree.h>
#include <zephyr.h>
#include <drivers/counter.h>
#include <drivers/timer/nrf_rtc_timer.h>

#include "rtc.h"

/* RTC device */
#define MY_RTC DT_PATH(soc, rtc_4000b000)
/* global variables for the counter */
static int32_t channel = 0;

void rtc_init(void)
{
    /* allocate channel for timer */
    channel =  z_nrf_rtc_timer_chan_alloc();
    printk("Allocated channel %d for a timer\n", channel);

    /* disable interrupt */
    bool locked = z_nrf_rtc_timer_compare_int_lock(channel);
    printk("Lock state: %i\n", locked);
}

uint64_t rtc_get_timestamp(void)
{
    return z_nrf_rtc_timer_read();
}