/*
 * Copyright (C) 2019 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "at45db.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

/**
 * @brief   the SPI mode to be used
 */
#define SPI_MODE        (SPI_MODE_0)

/**
 * @brief   Commands
 * @{
 */
#define CMD_FLASH_TO_BUF1       0x53    /**< Flash page to buffer 1 transfer */
#define CMD_FLASH_TO_BUF2       0x55    /**< Flash page to buffer 2 transfer */
#define CMD_READ_SECURITY_REGISTER 0x77 /**< Read Security Register */
#define CMD_PAGE_ERASE          0x81    /**< Page erase */
#define CMD_READ_MFGID          0x9F    /**< Read Manufacturer and Device ID */
#define CMD_BUF1_READ           0xD4    /**< Buffer 1 read */
#define CMD_BUF2_READ           0xD6    /**< Buffer 2 read */
#define CMD_READ_STATUS         0xD7    /**< Read Status Register */
/** @} */

#define MANUF_ADESTO            0x1F    /**< Manufacturer Adesto */
#define FAM_CODE_AT45D          0x01    /**< AT45Dxxx Family */


static inline void lock(at45db_t *dev)
{
    spi_acquire(dev->params.spi, dev->params.cs, SPI_MODE, dev->params.clk);
}

static inline void done(at45db_t *dev)
{
    spi_release(dev->params.spi);
}
static void check_id(const at45db_t *dev);
static uint16_t get_full_status(const at45db_t *dev);

int at45db_init(at45db_t *dev, const at45db_params_t *params)
{
    int retval;

    dev->params = *params;

    /* initialize SPI */
    retval = spi_init_cs(dev->params.spi, dev->params.cs);
    if (retval != SPI_OK) {
        return retval;
    }
    DEBUG("done initializing SPI master\n");
    check_id(dev);
    uint16_t status = get_full_status(dev);
    DEBUG("AT45DB: status = %04X\n", status);    

    return AT45DB_OK;
}


/**
 * @brief   Read the Manufacturer and Device ID
 *
 * @param[in] dev           device descriptor
 *
 */
static void check_id(const at45db_t *dev)
{
    uint8_t mfdid[4];
    uint8_t extdinfo[4];
    size_t ext_len;

    lock(dev);

    spi_transfer_byte(dev->params.spi, dev->params.cs, true, CMD_READ_MFGID);
    spi_transfer_bytes(dev->params.spi, dev->params.cs, true, NULL, mfdid, sizeof(mfdid));
    /* The fourth byte is the length of the Extended Device Information. */
    ext_len = mfdid[3];
    if (ext_len > sizeof(extdinfo)) {
        ext_len = sizeof(extdinfo);
    }
    if (ext_len == 0) {
        ext_len = 1;
    }
    spi_transfer_bytes(dev->params.spi, dev->params.cs, false, NULL, extdinfo, ext_len);
    DEBUG("AT45DB: Expecting 1F 2600\n");
    DEBUG("AT45DB: Manuf ID: %02X\n", mfdid[0]);
    DEBUG("AT45DB: Device ID: %02X%02X\n", mfdid[1], mfdid[2]);
    DEBUG("AT45DB:   Fam Code: %02X\n", (mfdid[1] >> 5) & 0x07);
    DEBUG("AT45DB:   Dens Code: %02X\n", mfdid[1] & 0x1F);
    DEBUG("AT45DB:   Sub Code: %02X\n", (mfdid[2] >> 5) & 0x07);
    DEBUG("AT45DB:   Prod Var: %02X\n", mfdid[2] & 0x1F);

    done(dev);

    /* Sanity Checks */

    /* Manufacturer */
    if (mfdid[0] != MANUF_ADESTO) {
        /* TODO */
    }

    /* Flash size */
    if ((mfdid[1] & 0x1F) != dev->params.details.density_code) {
        /* TODO */
    }
}

/**
 * @brief Get the full status for AT45DB161E (Adesto)
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 */
static uint16_t get_full_status(const at45db_t *dev)
{
    uint16_t status;
    lock(dev);
    spi_transfer_byte(dev->params.spi, dev->params.cs, true, CMD_READ_STATUS);
    spi_transfer_bytes(dev->params.spi, dev->params.cs, false, NULL, &status, sizeof(status));
    done(dev);
    return status;
}
