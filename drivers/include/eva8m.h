/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_eva8m u-blox EVA 8/8M
 * @ingroup     drivers_gps
 * @brief       Device driver interface for the u-blox EVA 8/8M series.
 *
 *
 * @{
 * @file
 * @brief       Device driver interface for the u-blox EVA 8/8M series.
 *
 * @details     TODO
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifndef EVA8M_H
#define EVA8M_H

#include <inttypes.h>
#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Parameters for the u-blox EVA 8/8M series.
 *
 * These parameters are needed to configure the device at startup.
 */
typedef struct {
    /* I2C details */
    i2c_t i2c_dev;                      /**< I2C device which is used */
    uint8_t i2c_addr;                   /**< I2C address */
} eva8m_params_t;

/**
 * @brief   Device descriptor for the EVA 8/8M
 */
typedef struct {
   eva8m_params_t params;               /**< Device Parameters */
} eva8m_t;

/**
 * @brief   Status and error return codes
 */
enum {
    EVA8M_OK           =  0,            /**< everything was fine */
    EVA8M_ERR_I2C      = -1,            /**< error initializing the I2C bus */
    EVA8M_ERR_NODEV    = -2,            /**< did not detect EVA 8 or 8M */
};

/**
 * @brief   Initialize the given EVA8M device
 *
 * @param[out] dev          Initialized device descriptor of EVA8M device
 * @param[in]  params       The parameters for the EVA8M device (sampling rate, etc)
 *
 * @return                  EVA8M_OK on success
 * @return                  EVA8M_ERR_I2C
 * @return                  EVA8M_ERR_NODEV
 */
int eva8m_init(eva8m_t* dev, const eva8m_params_t* params);

#ifdef __cplusplus
}
#endif

#endif /* EVA8M_H */
/** @} */
