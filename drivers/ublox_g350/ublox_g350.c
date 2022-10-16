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
 *
 * @file
 * @brief       Device driver implementation for the ublox_g350
 *
 * @author      Kees Bakker <kees@ijzerbout.nl>
 *
 * @}
 */

#include "ublox_g350.h"
#include "ublox_g350_constants.h"
#include "ublox_g350_params.h"
#include "ublox_g350_netdev.h"

void ublox_g350_setup(ublox_g350_t *dev, const ublox_g350_params_t *params, uint8_t index)
{
    netdev_t *netdev = (netdev_t *)dev;

    netdev->driver = &ublox_g350_driver;
    dev->params = (ublox_g350_params_t *)params;
    netdev_register(&dev->netdev, NETDEV_UBLOX_G350, index);
}

int ublox_g350_init(ublox_g350_t *dev)
{
    /* Initialize peripherals, gpios, setup registers, etc */
    (void)dev;      /* TODO */

    return 0;
}
