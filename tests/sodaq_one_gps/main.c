/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       SODAQ ONE GPS
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <stdio.h>

#include "board.h"
#include "periph/gpio.h"
#include "periph_conf.h"

static void cb(void *arg)
{
    (void)arg;
    printf("GPS Timepulse\n");
}

static void cb2(void *arg)
{
    (void)arg;
    printf("BTN0\n");
}

int main(void)
{
    puts("SODAQ ONE GPS");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    /* Switch on the device */
    GPS_ENABLE_ON;

    if (gpio_init_int(GPS_TIMEPULSE_PIN, GPS_TIMEPULSE_MODE, GPIO_RISING, cb, (void *)1) < 0) {
        puts("[FAILED] init GPS_TIMEPULSE!");
        return 1;
    }

    if (gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_FALLING, cb2, (void *)1) < 0) {
        puts("[FAILED] init BTN0!");
        return 1;
    }

    return 0;
}
