/*
 * Copyright (C) 2019 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */


/**
 * @ingroup     drivers_at45db
 *
 * @{
 * @file
 * @brief       Default configuration for AT45DB
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifndef AT45DB_PARAMS_H
#define AT45DB_PARAMS_H

#include "at45db.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Set default configuration parameters for the AT45DB
 * @{
 */
#ifndef AT45DB_PARAM_SPI_DEV
#define AT45DB_PARAM_SPI_DEV         SPI_DEV(0)
#endif
#ifndef AT45DB_PARAM_SPI_CS
#define AT45DB_PARAM_SPI_CS          GPIO_PIN(0,23)
#endif
#ifndef AT45DB_PARAM_SPI_CLK
#define AT45DB_PARAM_SPI_CLK         SPI_CLK_10MHZ
#endif
#ifndef AT45DB_PARAM_VARIANT
#define AT45DB_PARAM_VARIANT         AT45DB161E
#endif

#define AT45DB_PARAMS_DEFAULT        {.spi = AT45DB_PARAM_SPI_DEV,  \
                                      .cs = AT45DB_PARAM_SPI_CS, \
                                      .clk = AT45DB_PARAM_SPI_CLK, \
                                      .variant = AT45DB_PARAM_VARIANT }
/**@}*/

/**
 * @brief   Configure AT45DB
 */
static const at45db_params_t at45db_params[] =
{
#ifdef AT45DB_PARAMS_BOARD
    AT45DB_PARAMS_BOARD,
#else
    AT45DB_PARAMS_DEFAULT,
#endif
};

/**
 * @brief   Get the number of configured AT45DB devices
 */
#define AT45DB_NUMOF       (sizeof(at45db_params) / sizeof(at45db_params[0]))

#ifdef __cplusplus
}
#endif

#endif /* AT45DB_PARAMS_H */
/** @} */
