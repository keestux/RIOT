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
 * @brief       Driver for AT45DB using mtd interface
 * @{
 *
 * @file
 * @brief       Interface definition for mtd_at45db driver
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#include <errno.h>

#include "mtd_at45db.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static int mtd_at45db_init(mtd_dev_t *mtd_dev);
static int mtd_at45db_read(mtd_dev_t *mtd_dev,
                           void *dest, uint32_t addr, uint32_t size);
static int mtd_at45db_write(mtd_dev_t *mtd_dev,
                            const void *src, uint32_t addr, uint32_t size);
static int mtd_at45db_erase(mtd_dev_t *mtd_dev, uint32_t addr, uint32_t size);
static int mtd_at45db_power(mtd_dev_t *mtd_dev, enum mtd_power_state power);

const mtd_desc_t mtd_at45db_driver = {
    .init = mtd_at45db_init,
    .read = mtd_at45db_read,
    .write = mtd_at45db_write,
    .erase = mtd_at45db_erase,
    .power = mtd_at45db_power,
};

static int mtd_at45db_init(mtd_dev_t *mtd_dev)
{
    DEBUG("mtd_at45db_init\n");
    (void)mtd_dev;
    return -ENOTSUP;            /* TODO */
}

static int mtd_at45db_read(mtd_dev_t *mtd_dev,
                           void *dest, uint32_t addr, uint32_t size)
{
    DEBUG("mtd_at45db_read\n");
    (void)mtd_dev;
    (void)dest;
    (void)addr;
    (void)size;
    return -ENOTSUP;            /* TODO */
}

static int mtd_at45db_write(mtd_dev_t *mtd_dev,
                            const void *src, uint32_t addr, uint32_t size)
{
    DEBUG("mtd_at45db_write\n");
    (void)mtd_dev;
    (void)src;
    (void)addr;
    (void)size;
    return -ENOTSUP;            /* TODO */
}

static int mtd_at45db_erase(mtd_dev_t *mtd_dev, uint32_t addr, uint32_t size)
{
    DEBUG("mtd_at45db_erase\n");
    (void)mtd_dev;
    (void)addr;
    (void)size;
    return -ENOTSUP;            /* TODO */
}

static int mtd_at45db_power(mtd_dev_t *mtd_dev, enum mtd_power_state power)
{
    DEBUG("mtd_at45db_power\n");
    (void)mtd_dev;
    (void)power;
    return -ENOTSUP;            /* TODO */
}
