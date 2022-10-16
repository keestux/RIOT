/*
 * Copyright (C) 2022 IJzerbout IT
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ublox_g350
 * @{
 * @file
 * @brief       Netdev driver definitions for ublox_g350 driver
 *
 * @author      Kees Bakker <kees@ijzerbout.nl>
 */

#ifndef UBLOX_G350_NETDEV_H
#define UBLOX_G350_NETDEV_H

#include "net/netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Reference to the netdev device driver struct
 */
extern const netdev_driver_t ublox_g350_driver;

#ifdef __cplusplus
}
#endif

#endif /* UBLOX_G350_NETDEV_H */
/** @} */
