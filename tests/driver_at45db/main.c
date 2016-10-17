/*
 * Copyright (C) 2016 Kees Bakker, SODAQ
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
 * @brief       Test application for the AT45DB driver
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "board.h"
#include "xtimer.h"
#include "periph/spi.h"
#include "at45db.h"

#ifdef TEST_AT45DB_SPI_CONF
#define SPI_CONF    (TEST_AT45DB_SPI_CONF)
#else
#define SPI_CONF    (SPI_CONF_FIRST_RISING)
#endif

#ifdef TEST_AT45DB_SPI_SPEED
#define SPI_SPEED   (TEST_AT45DB_SPI_SPEED)
#else
#define SPI_SPEED   (SPI_SPEED_10MHZ)
#endif

int main(void)
{
    at45db_t dev;

    puts("AT45DB test application starting...");

    printf("Initializing SPI_%i... ", TEST_AT45DB_SPI_DEV);
    if (spi_init_master(TEST_AT45DB_SPI_DEV, SPI_CONF, TEST_AT45DB_SPI_SPEED) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]\n");
        return 1;
    }

    puts("Initializing AT45DB device descriptor... ");
    if (at45db_init(&dev, TEST_AT45DB_SPI_DEV, TEST_AT45DB_SPI_CS) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]\n");
        return 1;
    }

    while(1);

    return 0;
}
