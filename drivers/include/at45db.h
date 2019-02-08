/*
 * Copyright (C) 2019 Kees Bakker, SODAQ
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

typedef enum {
    AT45DB161E,                 /**< 16Mbit, 4096 pages of 526 bytes */
    AT45DB641E,                 /**< 64Mbit, 32768 pages of 268 bytes */
} at45db_variant_t;

/**
 * @brief   Chip details AT45DB series
 */
typedef struct at45db_chip_details_s {
    size_t page_addr_bits;      /**< Number of bits for a page address */
    size_t nr_pages;            /**< Number of pages, must be (1 << page_addr_bits) */
    size_t page_size;           /**< Size of a page */
    size_t page_size_alt;       /**< Alternative size of a page */
    size_t page_size_bits;      /**< Number of bits to address inside a page */
    uint8_t density_code;       /**< The density code in byte 1 Device Details */
} at45db_chip_details_t;

/**
 * @brief   Device auto initialization parameters
 */
typedef struct {
    spi_t spi;                  /**< SPI bus the dataflash is connected to */
    spi_cs_t cs;                /**< SPI chip select pin */
    spi_clk_t clk;              /**< SPI bus clock speed */
    at45db_variant_t variant;   /**< chip variant */
} at45db_params_t;

/**
 * @brief   Device descriptor for the AT45DB series data flash
 */
typedef struct {
    at45db_params_t params;     /**< parameters for initialization */
    const at45db_chip_details_t *details;  /**< chip details */
} at45db_t;

/**
 * @brief   AT45DB return code
 */
enum {
    AT45DB_OK = 0,                  /**< success */
    AT45DB_ERROR = -21,             /**< generic error */
    AT45DB_UNKNOWN_VARIANT = -22,   /**< unknown chip variant */
};

/**
 * @brief Initialize the given AT45DB device
 *
 * @param[out] dev          Initialized device descriptor of AT45DB device
 * @param[in]  params       
 *
 * @return                  AT45DB_OK on success
 * @return                  SPI_NODEV on invalid device
 * @return                  SPI_NOCS on invalid CS pin/line
 */
int at45db_init(at45db_t *dev, const at45db_params_t *params);

/**
 * @brief Read the Security Register
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[out] data         Pointer to the destination buffer
 * @param[in]  data_size    Size of the data
 *
 * @return                  AT45DB_OK on success
 * @return                  -AT45DB_ERROR error
 */
int at45db_security_register(const at45db_t *dev, uint8_t *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* AT45DB_H_ */
/** @} */
