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
 * @brief Chip details AT45DB series
 */
typedef struct {
    size_t page_addr_bits;      /**< Number of bits for a page address */
    size_t nr_pages;            /**< Number of pages, must be (1 << page_addr_bits) */
    size_t page_size;           /**< Size of a page */
    size_t page_size_alt;       /**< Alternative size of a page */
    size_t page_size_bits;      /**< Number of bits to address inside a page */
    uint8_t density_code;       /**< The density code in byte 1 Device Details */
} at45db_chip_details_t;
extern const at45db_chip_details_t at45db161e;
extern const at45db_chip_details_t at45db641e;

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
 * @brief Device initialization parameters
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
 *
 * @return                  0 on success
 * @return                  -1 if given SPI is not enabled in board config
 */
int at45db_init(at45db_t *dev, spi_t spi, gpio_t cs, const at45db_chip_details_t *details);

/**
 * @brief Read data from buffer
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  bufno        The dataflash buffer number (can only be 1 or 2)
 * @param[out] data         Pointer to the destination buffer
 * @param[out] data_size    Size of the data
 *
 * @return                  0 on success
 * @return                  -1 invalid buffer number
 * @return                  -2 data could not be loaded
 */
int at45db_read_buf(at45db_t *dev, size_t bufno, uint8_t *data, size_t data_size);

/**
 * @brief Read page into df buffer
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  page         The dataflash page number
 * @param[in]  bufno        The dataflash buffer number (can only be 1 or 2)
 *
 * @return                  0 on success
 * @return                  -1 invalid buffer number
 * @return                  -2 invalid page number
 * @return                  -3 data not read
 */
int at45db_page2buf(at45db_t *dev, size_t page, size_t bufno);

/**
 * @brief Erase a page
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 * @param[in]  page         The dataflash page number
 *
 * @return                  0 on success
 * @return                  -2 invalid page number
 */
int at45db_erase_page(at45db_t *dev, size_t page);

#ifdef __cplusplus
}
#endif

#endif /* AT45DB_H_ */
/** @} */
