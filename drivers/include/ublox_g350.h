/*
 * Copyright (C) 2022 IJzerbout IT
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ublox_g350 ublox_g350
 * @ingroup     drivers_netdev
 * @brief       A netdev driver for Ublox G350
 *
 * @{
 *
 * @file
 *
 * @author      Kees Bakker <kees@ijzerbout.nl>
 */

#ifndef UBLOX_G350_H
#define UBLOX_G350_H

#include "net/netdev.h"
/* Add header includes here */

#ifdef __cplusplus
extern "C" {
#endif

/* Declare the API of the driver */

/**
 * @brief   Device initialization parameters
 */
typedef struct {
    /* add initialization params here */
} ublox_g350_params_t;

/**
 * @brief   Device descriptor for the driver
 */
typedef struct {
    netdev_t netdev;                        /**< Netdev parent struct */
    /** Device initialization parameters */
    ublox_g350_params_t *params;
} ublox_g350_t;

/**
 * @brief   Setup the device structure and register it to netdev
 *
 * @param[in] dev                       Device descriptor
 * @param[in] params                    Parameters for device initialization
 * @param[in] index                     Index of @p params in a global parameter struct array.
 *                                      If initialized manually, pass a unique identifier instead.
 */
void ublox_g350_setup(ublox_g350_t *dev, const ublox_g350_params_t *params, uint8_t index);

/**
 * @brief   Initialize the given device
 *
 * @param[inout] dev                    Device descriptor of the driver
 *
 * @return                  0 on success
 */
int ublox_g350_init(ublox_g350_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* UBLOX_G350_H */
/** @} */
