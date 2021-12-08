/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <zephyr.h>

#include "handlers.h"

void main(void)
{
	wifi_handler_connect(true);
	bt_handler_init();
	
}