/*
 * Copyright (C) 2016 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_at45db AT45DB
 * @ingroup     drivers_dataflash
 * @brief       Device driver interface for the AT45DB dataflash
 * @{
 *
 * @file
 * @brief       Device driver interface for the AT45DB dataflash.
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifndef AT45DB_H_
#define AT45DB_H_

#include "periph/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Device descriptor for the AT45DB sensor
 */
typedef struct {
    spi_t spi;              /**< SPI device which is used */
    spi_speed_t spi_speed;  /**< SPI speed to use */
} at45db_t;

/**
 * @brief Device initialization parameters
 */
typedef struct {
    spi_t spi;
    spi_speed_t spi_speed;  /**< SPI speed to use */
} at45db_params_t;

/**
 * @brief auto-initialize all configured AT45DB devices
 */
void at45db_auto_init(void);

/**
 * @brief Initialize the given AT45DB device
 *
 * @param[out] dev          Initialized device descriptor of AT45DB device
 * @param[in]  spi          SPI bus the sensor is connected to
 *
 * @return                  0 on success
 * @return                  -1 if given SPI is not enabled in board config
 */
int at45db_init(at45db_t *dev, spi_t spi);

#ifdef __cplusplus
}
#endif

#endif /* AT45DB_H_ */
/** @} */
