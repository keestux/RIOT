/*
 * Copyright (C) 2016 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_sodaq-autonomo
 * @{
 *
 * @file
 * @brief       Board specific implementations for the SODAQ Autonomo
 *              Pro board
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include "board.h"
#include "periph/gpio.h"

void board_init(void)
{
    /* initialize the on-board LED */
    gpio_init(LED0_PIN, GPIO_OUT);

    BEE_VCC_OFF;
    gpio_init(BEE_VCC_PIN, GPIO_OUT);

    BEE_DTR_OFF;
    gpio_init(BEE_DTR_PIN, GPIO_OUT);

    VCC_SW_OFF;
    gpio_init(VCC_SW_PIN, GPIO_OUT);

    /* initialize the CPU */
    cpu_init();
}
