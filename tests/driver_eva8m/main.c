/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the u-blox EVA 8/8M.
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <stdio.h>
#include <inttypes.h>

#include "eva8m_params.h"
#include "eva8m.h"
#include "xtimer.h"

#define MAINLOOP_DELAY  (2 * 1000 * 1000u)      /* 2 seconds delay between printf's */

int main(void)
{
    eva8m_t dev;
    int result;

    puts("EVA8M test application\n");

    GPS_ENABLE_ON;

    result = eva8m_init(&dev, &eva8m_params[0]);
    if (result == EVA8M_ERR_I2C) {
        puts("[Error] The given i2c is not enabled");
        return 1;
    }
    if (result == EVA8M_ERR_NODEV) {
        puts("[Error] Did not detect an EVA 8/8M");
        return 1;
    }

    while (1) {
        xtimer_usleep(MAINLOOP_DELAY);
    }
}
