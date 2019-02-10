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
 * @{
 *
 * @file
 * @brief       Driver for AT45DB using mtd interface
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <errno.h>

#include "mtd_at45db.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static int mtd_at45db_init(mtd_dev_t *mtd_base);
static int mtd_at45db_read(mtd_dev_t *mtd_base,
                           void *dest, uint32_t addr, uint32_t size);
static int mtd_at45db_write(mtd_dev_t *mtd_base,
                            const void *src, uint32_t addr, uint32_t size);
static int mtd_at45db_erase(mtd_dev_t *mtd_base, uint32_t addr, uint32_t size);
static int mtd_at45db_power(mtd_dev_t *mtd_base, enum mtd_power_state power);

const mtd_desc_t mtd_at45db_driver = {
    .init = mtd_at45db_init,
    .read = mtd_at45db_read,
    .write = mtd_at45db_write,
    .erase = mtd_at45db_erase,
    .power = mtd_at45db_power,
};

static int mtd_at45db_init(mtd_dev_t *mtd_base)
{
    DEBUG("mtd_at45db_init\n");
    mtd_at45db_t *mtd_dev = (mtd_at45db_t*)mtd_base;
    at45db_t *dev = mtd_dev->at45db_dev;
    if (!dev->init_done) {
        int res;
        res = at45db_init(dev, mtd_dev->params);
        if (res != AT45DB_OK) {
            return res;
        }
    } else {
        DEBUG("[mtd_at45db_init] dev already initialized\n");
    }
    mtd_dev->base.page_size = dev->details->page_size;
    mtd_dev->base.pages_per_sector = 1;
    mtd_dev->base.sector_count = dev->details->nr_pages;
    DEBUG("[mtd_at45db_init] nr sectors: %lu\n", mtd_dev->base.sector_count);
    DEBUG("[mtd_at45db_init] page size: %lu\n", mtd_dev->base.page_size);
    return AT45DB_OK;
 }

static int mtd_at45db_read(mtd_dev_t *mtd_base,
                           void *dest, uint32_t addr, uint32_t size)
{
    DEBUG("mtd_at45db_read: addr:%" PRIu32 " size:%" PRIu32 "\n", addr, size);
    mtd_at45db_t *mtd_dev = (mtd_at45db_t*)mtd_base;
    at45db_t *dev = mtd_dev->at45db_dev;
    int res;
    if ((addr % mtd_dev->base.page_size) != 0) {
        /* Not sure if this can happen. */
        DEBUG("[mtd_at45db_read] Not aligned start\n");
        return -ENOTSUP;
    }
    if ((size % mtd_dev->base.page_size) != 0) {
        /* Not sure if this can happen. */
        DEBUG("[mtd_at45db_read] Not aligned size\n");
        return -ENOTSUP;
    }
    uint32_t start_page = addr / mtd_dev->base.page_size;
    uint32_t nr_pages = size / mtd_dev->base.page_size;
    uint32_t remaining_size = size;
    for (size_t i = 0; i < nr_pages; i++) {
        uint32_t page_nr = start_page + i;
        uint8_t *dest_buf = ((uint8_t *)dest) + i * mtd_dev->base.page_size;
        uint32_t len = remaining_size;
        if (len > mtd_dev->base.page_size) {
            len = mtd_dev->base.page_size;
        }
        res = at45db_read_page(dev, page_nr, dest_buf, len);
        if (res != AT45DB_OK) {
            return res;
        }
        remaining_size -= len;
    }
    return (int)(size - remaining_size);
}

static int mtd_at45db_write(mtd_dev_t *mtd_base,
                            const void *src, uint32_t addr, uint32_t size)
{
    DEBUG("mtd_at45db_write\n");
    (void)mtd_base;
    (void)src;
    (void)addr;
    (void)size;
    return -ENOTSUP;            /* TODO */
}

static int mtd_at45db_erase(mtd_dev_t *mtd_base, uint32_t addr, uint32_t size)
{
    DEBUG("mtd_at45db_erase\n");
    (void)mtd_base;
    (void)addr;
    (void)size;
    return -ENOTSUP;            /* TODO */
}

static int mtd_at45db_power(mtd_dev_t *mtd_base, enum mtd_power_state power)
{
    DEBUG("mtd_at45db_power\n");
    (void)mtd_base;
    (void)power;
    return -ENOTSUP;            /* TODO */
}
