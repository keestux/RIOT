/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_eva8m
 *
 * @{
 * @file
 * @brief       Default configuration for EVA 8/8M
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifndef EVA8M_PARAMS_H
#define EVA8M_PARAMS_H

#include "board.h"
#include "eva8m.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the EVA8M
 * @{
 */
#ifndef EVA8M_PARAM_I2C_DEV
#define EVA8M_PARAM_I2C_DEV         I2C_DEV(0)
#endif
#ifndef EVA8M_PARAM_I2C_ADDR
#define EVA8M_PARAM_I2C_ADDR        (0x42)
#endif

/* Defaults for Weather Monitoring */
#define EVA8M_PARAMS_DEFAULT                    \
    {                                           \
        .i2c_dev = EVA8M_PARAM_I2C_DEV,         \
        .i2c_addr = EVA8M_PARAM_I2C_ADDR,       \
    }
/**@}*/

/**
 * @brief   Configure EVA8M
 */
static const eva8m_params_t eva8m_params[] =
{
#ifdef EVA8M_PARAMS_BOARD
    EVA8M_PARAMS_BOARD,
#else
    EVA8M_PARAMS_DEFAULT
#endif
};

/**
 * @brief   The number of configured sensors
 */
#define EVA8M_NUMOF    (sizeof(eva8m_params) / sizeof(eva8m_params[0]))

#ifdef __cplusplus
}
#endif

#endif /* EVA8M_PARAMS_H */
/** @} */
