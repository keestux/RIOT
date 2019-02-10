/*
 * Copyright (C) 2019 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_at45db AT45DB
 * @ingroup     drivers_storage
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

#include <stdbool.h>
#include <stdint.h>

#include "periph/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Chip variants
 * @details The AT45DB manufactured by Adesto (originally by Atmel)
 */
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
    bool init_done;             /**< flag to indicate that the init function was done */
} at45db_t;

/**
 * @brief   AT45DB return code
 */
enum {
    AT45DB_OK = 0,                  /**< success */
    AT45DB_ERROR = -21,             /**< generic error */
    AT45DB_UNKNOWN_VARIANT = -22,   /**< unknown chip variant */
    AT45DB_INVALID_BUFNR = -23,     /**< invalid buffer number */
    AT45DB_INVALID_PAGENR = -24,    /**< invalid page number */
};

/**
 * @brief   Initialize the given AT45DB device
 *
 * @param[out] dev          Initialized device descriptor of AT45DB device
 * @param[in]  params       Configuration parameters (SPI dev, CS pin, etc)
 *
 * @return                  AT45DB_OK on success
 * @return                  SPI_NODEV on invalid device
 * @return                  SPI_NOCS on invalid CS pin/line
 */
int at45db_init(at45db_t *dev, const at45db_params_t *params);

/**
 * @brief   Read a page from the device
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  pagenr       The page number
 * @param[out] data         Pointer to the destination buffer
 * @param[in]  len          The number of bytes to read
 *
 * @return                  AT45DB_OK on success
 * @return                  AT45DB_ERROR error
 */
int at45db_read_page(const at45db_t *dev, uint32_t pagenr, uint8_t *data, size_t len);

/**
 * @brief   Read a page from the device
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  pagenr       The page number
 * @param[in]  bufnr        The at45db buffer number
 *
 * @return                  AT45DB_OK on success
 * @return                  AT45DB_INVALID_BUFNR not a valid buffer number
 * @return                  AT45DB_INVALID_PAGENR not a valid page number
 */
int at45db_page2buf(const at45db_t *dev, size_t pagenr, size_t bufnr);

int at45db_read_buf(const at45db_t *dev, size_t bufnr, size_t start, uint8_t *data, size_t len);

/**
 * @brief   Read the Security Register
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[out] data         Pointer to the destination buffer
 * @param[in]  data_size    Size of the data
 *
 * @return                  AT45DB_OK on success
 * @return                  AT45DB_ERROR error
 */
int at45db_security_register(const at45db_t *dev, uint8_t *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* AT45DB_H_ */
/** @} */
