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

typedef enum {
    AT45DB161E,                 /**< 16Mbit, 4096 pages of 526 bytes */
    AT45DB641E,                 /**< 64Mbit, 32768 pages of 268 bytes */
} at45db_variant_t;

/**
 * @brief Chip details AT45DB series
 */
typedef struct at45db_chip_details_s at45db_chip_details_t;

/**
 * @brief Device descriptor for the AT45DB series data flash
 */
typedef struct {
    spi_t spi;                  /**< SPI device which is used */
    spi_cs_t cs;
    spi_clk_t clk;              /**< SPI clock (speed) to use */
    const at45db_chip_details_t *details;
} at45db_t;

/**
 * @brief Device auto initialization parameters
 */
typedef struct {
    spi_t spi;
    spi_cs_t cs;
    spi_clk_t clk;
    const at45db_chip_details_t *details;
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
 * @param[in]  cs           GPIO pin that is connected to the CS pin
 * @param[in]  variant      The chip variant
 *
 * @return                  0 on success
 * @return                  -1 given SPI is not enabled in board config
 * @return                  -2 chip variant was not selected, or unkown
 */
int at45db_init(at45db_t *dev, spi_t spi, gpio_t cs, at45db_variant_t variant);

/**
 * @brief Read data from buffer
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  bufno        The dataflash buffer number (can only be 1 or 2)
 * @param[in]  start        The start address in the dataflash buffer
 * @param[out] data         Pointer to the destination buffer
 * @param[in]  data_size    Size of the data
 *
 * @return                  0 on success
 * @return                  -1 invalid buffer number
 * @return                  -2 data could not be loaded
 */
int at45db_read_buf(at45db_t *dev, size_t bufno, size_t start, uint8_t *data, size_t data_size);

/**
 * @brief Read page into df buffer
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  pagenr       The dataflash page number
 * @param[in]  bufno        The dataflash buffer number (can only be 1 or 2)
 *
 * @return                  0 on success
 * @return                  -1 invalid buffer number
 * @return                  -2 invalid page number
 * @return                  -3 data not read
 */
int at45db_page2buf(at45db_t *dev, size_t pagenr, size_t bufno);

/**
 * @brief Erase a page
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  pagenr       The dataflash page number
 *
 * @return                  0 on success
 * @return                  -2 invalid page number
 */
int at45db_erase_page(at45db_t *dev, size_t pagenr);

/**
 * @brief Get page size of the selected AT45DB
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 *
 * @returns                 The page size, value 0 indicates unknown
 */
size_t at45db_get_page_size(at45db_t *dev);

/**
 * @brief Get number of pages of the selected AT45DB
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 *
 * @returns                 The number of pages, value 0 indicates unknown
 */
size_t at45db_get_nr_pages(at45db_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* AT45DB_H_ */
/** @} */
