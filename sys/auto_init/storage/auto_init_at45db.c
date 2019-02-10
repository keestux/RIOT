/*
 * Copyright (C) 2019 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_auto_init
 * @{
 *
 * @file
 * @brief       Auto initialization for AT45DB SPI devices
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifdef MODULE_AT45DB

#include "log.h"
#include "at45db.h"
#include "at45db_params.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/**
 * @brief   number of used sd cards
 * @{
 */
#define AT45DB_NUM (sizeof(at45db_params) / sizeof(at45db_params[0]))
/** @} */

/**
 * @brief   Allocate memory for the device descriptors
 * @{
 */
at45db_t at45db_devs[AT45DB_NUM];
/** @} */

void auto_init_at45db(void)
{
    for (unsigned i = 0; i < AT45DB_NUM; i++) {
        LOG_DEBUG("[auto_init_storage] initializing at45db #%u\n", i);

        if (at45db_init(&at45db_devs[i], &at45db_params[i]) !=
            AT45DB_OK) {
            LOG_ERROR("[auto_init_storage] error initializing at45db #%u\n", i);
        }
    }
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_AT45DB */
/** @} */
