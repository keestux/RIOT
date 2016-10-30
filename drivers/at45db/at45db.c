/*
 * Copyright (C) 2016 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "log.h"
#include "periph/spi.h"
#include "xtimer.h"

#include "at45db.h"
#include "at45db_params.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

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

/**
 * @brief Chip details AT45DB series
 */
struct at45db_chip_details_s {
    size_t page_addr_bits;      /**< Number of bits for a page address */
    size_t nr_pages;            /**< Number of pages, must be (1 << page_addr_bits) */
    size_t page_size;           /**< Size of a page */
    size_t page_size_alt;       /**< Alternative size of a page */
    size_t page_size_bits;      /**< Number of bits to address inside a page */
    uint8_t density_code;       /**< The density code in byte 1 Device Details */
};

static const at45db_chip_details_t at45db161e = {
    .page_addr_bits = 12,
    .nr_pages = 4096,
    .page_size = 528,
    .page_size_alt = 512,
    .page_size_bits = 10,
    .density_code = 0x6,
};
static const at45db_chip_details_t at45db641e = {
    .page_addr_bits = 15,
    .nr_pages = 32768,
    .page_size = 264,
    .page_size_alt = 256,
    .page_size_bits = 9,
    .density_code = 0x8,
};

static const at45db_chip_details_t *at45db_select_variant(at45db_variant_t variant);

static void check_id(at45db_t *dev);
static inline bool is_valid_bufno(size_t bufno);
static inline bool is_valid_page(size_t pagenr, const at45db_chip_details_t *details);
static uint8_t get_page_addr_byte0(uint16_t pagenr, size_t shift);
static uint8_t get_page_addr_byte1(uint16_t pagenr, size_t shift);
static uint8_t get_page_addr_byte2(uint16_t pagenr, size_t shift);
static uint16_t get_full_status(at45db_t *dev);
static inline void wait_till_ready(at45db_t *dev);
static inline void lock(at45db_t *dev);
static inline void done(at45db_t *dev);

int at45db_init(at45db_t *dev, spi_t spi, gpio_t cs, at45db_variant_t variant)
{
    const at45db_chip_details_t *details;
    details = at45db_select_variant(variant);
    if (details == NULL) {
        return -2;
    }

    /* Save device details */
    dev->spi = spi;
    dev->cs = cs;
    dev->details = details;

    /* initialize SPI */
    spi_init_cs(spi, (spi_cs_t)cs);
    DEBUG("done initializing SPI master\n");
    check_id(dev);
    uint16_t status = get_full_status(dev);
    DEBUG("AT45DB: status = %04X\n", status);    

    return 0;
}

static const at45db_chip_details_t *at45db_select_variant(at45db_variant_t variant)
{
    const at45db_chip_details_t * retval = NULL;
    switch (variant) {
    case AT45DB161E:
        retval = &at45db161e;
        break;
    case AT45DB641E:
        retval = &at45db641e;
        break;
    default:
        break;
    }
    return retval;
}

int at45db_read_buf(at45db_t *dev, size_t bufno, size_t start, uint8_t *data, size_t data_size)
{
    uint8_t cmd;
    if (!is_valid_bufno(bufno)) {
        return -1;
    }
    if (data == NULL || data_size == 0) {
        /* Not sure if this makes sense, but it validates the function arguments */
        return 0;
    }

    cmd = bufno == 1 ? CMD_BUF1_READ : CMD_BUF2_READ;

    lock(dev);
    wait_till_ready(dev);
    spi_transfer_byte(dev->spi, dev->cs, true, cmd);
    spi_transfer_byte(dev->spi, dev->cs, true, 0x00);           /* don't care */
    spi_transfer_byte(dev->spi, dev->cs, true, start >> 8);     /* addr, ms byte */
    spi_transfer_byte(dev->spi, dev->cs, true, start);          /* addr, ls byte */
    spi_transfer_byte(dev->spi, dev->cs, true, 0x00);           /* don't care */
    spi_transfer_bytes(dev->spi, dev->cs, false, NULL, data, data_size);
    done(dev);

    return 0;
}

int at45db_page2buf(at45db_t *dev, size_t pagenr, size_t bufno)
{
    // DEBUG("AT45DB: page#%d to buf%d\n", pagenr, bufno);
    uint8_t cmd[4];
    if (!is_valid_bufno(bufno)) {
        return -1;
    }
    if (!is_valid_page(pagenr, dev->details)) {
        return -2;
    }
    cmd[0] = bufno == 1 ? CMD_FLASH_TO_BUF1 : CMD_FLASH_TO_BUF2;
    cmd[1] = get_page_addr_byte0(pagenr, dev->details->page_size_bits);
    cmd[2] = get_page_addr_byte1(pagenr, dev->details->page_size_bits);
    cmd[3] = get_page_addr_byte2(pagenr, dev->details->page_size_bits);
    // DEBUG("AT45DB: cmd=%02X%02X%02X%02X\n", cmd[0], cmd[1], cmd[2], cmd[3]);

    lock(dev);
    spi_transfer_bytes(dev->spi, dev->cs, false, cmd, NULL, sizeof(cmd));
    done(dev);

    return 0;
}

int at45db_erase_page(at45db_t *dev, size_t pagenr)
{
    DEBUG("AT45DB: erase page#%d\n", pagenr);
    uint8_t cmd[4];
    if (!is_valid_page(pagenr, dev->details)) {
        return -2;
    }
    cmd[0] = CMD_PAGE_ERASE;
    cmd[1] = get_page_addr_byte0(pagenr, dev->details->page_size_bits);
    cmd[2] = get_page_addr_byte1(pagenr, dev->details->page_size_bits);
    cmd[3] = get_page_addr_byte2(pagenr, dev->details->page_size_bits);
    DEBUG("AT45DB: cmd=%02X%02X%02X%02X\n", cmd[0], cmd[1], cmd[2], cmd[3]);

    lock(dev);
    spi_transfer_bytes(dev->spi, dev->cs, false, cmd, NULL, sizeof(cmd));
    wait_till_ready(dev);
    done(dev);
    return 0;
}

int at45db_security_register(at45db_t *dev, uint8_t *data, size_t data_size)
{
    uint8_t cmd;

    if (data == NULL || data_size == 0) {
        /* Not sure if this makes sense, but it validates the function arguments */
        return 0;
    }

    cmd = CMD_READ_SECURITY_REGISTER;
    lock(dev);
    wait_till_ready(dev);
    spi_transfer_byte(dev->spi, dev->cs, true, cmd);
    spi_transfer_byte(dev->spi, dev->cs, true, 0x00);           /* don't care */
    spi_transfer_byte(dev->spi, dev->cs, true, 0x00);           /* don't care */
    spi_transfer_byte(dev->spi, dev->cs, true, 0x00);           /* don't care */
    spi_transfer_bytes(dev->spi, dev->cs, false, NULL, data, data_size);
    done(dev);

    return 0;
}

size_t at45db_get_page_size(at45db_t *dev)
{
    return dev && dev->details ? dev->details->page_size : 0;
}

size_t at45db_get_nr_pages(at45db_t *dev)
{
    return dev && dev->details ? dev->details->nr_pages : 0;
}

/**
 * @brief   Read the Manufacturer and Device ID
 *
 * @param[in] dev           device descriptor
 *
 */
static void check_id(at45db_t *dev)
{
    uint8_t mfdid[4];
    uint8_t extdinfo[4];
    size_t ext_len;

    lock(dev);

    spi_transfer_byte(dev->spi, dev->cs, true, CMD_READ_MFGID);
    spi_transfer_bytes(dev->spi, dev->cs, true, NULL, mfdid, sizeof(mfdid));
    /* The fourth byte is the length of the Extended Device Information. */
    ext_len = mfdid[3];
    if (ext_len > sizeof(extdinfo)) {
        ext_len = sizeof(extdinfo);
    }
    if (ext_len == 0) {
        ext_len = 1;
    }
    spi_transfer_bytes(dev->spi, dev->cs, false, NULL, extdinfo, ext_len);
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
    if ((mfdid[1] & 0x1F) != dev->details->density_code) {
        /* TODO */
    }
}

static inline bool is_valid_bufno(size_t bufno)
{
    return bufno == 1 || bufno == 2;
}

static inline bool is_valid_page(size_t pagenr, const at45db_chip_details_t *details)
{
    return pagenr < details->nr_pages;
}

/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *    followed by three address bytes consist of three don’t care bits,
 *    12 page address bits (PA11 - PA0) that specify the page in the main
 *    memory to be written and nine don’t care bits."
 *
 *  32109876 54321098 76543210
 *  ---aaaaa aaaaaaa- --------
 */
/*
 * From the AT45DB161D documentation (AT45DB161E is identical)
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 *
 *  32109876 54321098 76543210
 *  --aaaaaa aaaaaa-- --------
 */
/*
 * From the AT45DB041D documentation
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *   followed by three address bytes consist of four don’t care bits, 11 page
 *   address bits (PA10 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 *
 *  32109876 54321098 76543210
 *  ----aaaa aaaaaaa- --------
 */
static uint8_t get_page_addr_byte0(uint16_t pagenr, size_t shift)
{
    // More correct would be to use a 24 bits number
    // shift to the left by number of bits. But the uint16_t can be considered
    // as if it was already shifted by 8.
    return (pagenr << (shift - 8)) >> 8;
}
static uint8_t get_page_addr_byte1(uint16_t pagenr, size_t shift)
{
    return pagenr << (shift - 8);
}
static uint8_t get_page_addr_byte2(uint16_t pagenr, size_t shift)
{
    return 0;
}

/**
 * @brief Get the full status for AT45DB161E (Adesto)
 *
 * @param[in]  dev          The device descriptor of AT45DB device
 */
static uint16_t get_full_status(at45db_t *dev)
{
    uint16_t status;
    lock(dev);
    spi_transfer_byte(dev->spi, dev->cs, true, CMD_READ_STATUS);
    spi_transfer_bytes(dev->spi, dev->cs, false, NULL, &status, sizeof(status));
    done(dev);
    return status;
}

static inline void wait_till_ready(at45db_t *dev)
{
    uint8_t status;
    spi_transfer_byte(dev->spi, dev->cs, true, CMD_READ_STATUS);
    do {
        status = spi_transfer_byte(dev->spi, dev->cs, true, 0);
    } while ((status & 0x80) == 0);
    spi_transfer_byte(dev->spi, dev->cs, false, 0);
}

static inline void lock(at45db_t *dev)
{
    spi_acquire(dev->spi, dev->cs, SPI_MODE, dev->clk);
}

static inline void done(at45db_t *dev)
{
    spi_release(dev->spi);
}
