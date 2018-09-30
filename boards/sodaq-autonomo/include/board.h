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
 * @brief       Board specific definitions for the SODAQ Autonomo board
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "periph_conf.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    xtimer configuration
 * @{
 */
#define XTIMER              TIMER_1
#define XTIMER_CHAN         (0)
/** @} */

/**
 * @name    LED pin definitions and handlers
 * @{
 */
#define LED0_PIN            GPIO_PIN(PA, 18)

#define LED_PORT            PORT->Group[PA]
#define LED0_MASK           (1 << 18)

#define LED0_ON             (LED_PORT.OUTSET.reg = LED0_MASK)
#define LED0_OFF            (LED_PORT.OUTCLR.reg = LED0_MASK)
#define LED0_TOGGLE         (LED_PORT.OUTTGL.reg = LED0_MASK)
/** @} */

/**
 * @name    Bee Vcc On/Off
 * @{
 */
#define BEE_VCC_PIN         GPIO_PIN(PA, 28)

#define BEE_VCC_PORT        PORT->Group[PA]
#define BEE_VCC_MASK        (1 << 28)

#define BEE_VCC_ON          (BEE_VCC_PORT.OUTSET.reg = BEE_VCC_MASK)
#define BEE_VCC_OFF         (BEE_VCC_PORT.OUTCLR.reg = BEE_VCC_MASK)
#define BEE_VCC_TOGGLE      (BEE_VCC_PORT.OUTTGL.reg = BEE_VCC_MASK)
/** @} */

/**
 * @name    Bee DTR
 * @{
 */
#define BEE_DTR_PIN         GPIO_PIN(PB, 1)

#define BEE_DTR_PORT        PORT->Group[PB]
#define BEE_DTR_MASK        (1 << 1)

#define BEE_DTR_ON          (BEE_DTR_PORT.OUTSET.reg = BEE_DTR_MASK)
#define BEE_DTR_OFF         (BEE_DTR_PORT.OUTCLR.reg = BEE_DTR_MASK)
#define BEE_DTR_TOGGLE      (BEE_DTR_PORT.OUTTGL.reg = BEE_DTR_MASK)
/** @} */

/**
 * @name    Vcc On/Off
 * @{
 */
#define VCC_SW_PIN          GPIO_PIN(PA, 8)

#define VCC_SW_PORT         PORT->Group[PA]
#define VCC_SW_MASK         (1 << 8)

#define VCC_SW_ON           (VCC_SW_PORT.OUTSET.reg = VCC_SW_MASK)
#define VCC_SW_OFF          (VCC_SW_PORT.OUTCLR.reg = VCC_SW_MASK)
#define VCC_SW_TOGGLE       (VCC_SW_PORT.OUTTGL.reg = VCC_SW_MASK)
/** @} */

/**
 * @brief   Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
