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

#ifndef MTD_AT45DB_H
#define MTD_AT45DB_H

#include "at45db.h"
#include "mtd.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Device descriptor for mtd_at45db device
 *
 * This is an extension of the @c mtd_dev_t struct
 */
typedef struct {
    mtd_dev_t base;                     /**< inherit from mtd_dev_t object */
    at45db_t *at45db_dev;               /**< at45db dev descriptor */
    const at45db_params_t *params;      /**< params for at45db init */
} mtd_at45db_t;

/**
 * @brief   AT45DB device operations table for mtd
 */
extern const mtd_desc_t mtd_at45db_driver;

#ifdef __cplusplus
}
#endif

#endif /* MTD_AT45DB_H */
/** @} */
