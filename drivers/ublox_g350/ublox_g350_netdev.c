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
 * @brief       Netdev adaptation for the ublox_g350 driver
 *
 * @author      Kees Bakker <kees@ijzerbout.nl>
 * @}
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "iolist.h"
#include "net/netopt.h"
#include "net/netdev.h"

#include "ublox_g350.h"
#include "ublox_g350_netdev.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static int _send(netdev_t *netdev, const iolist_t *iolist)
{
    ublox_g350_t *dev = (ublox_g350_t *)netdev;
    netopt_state_t state;
    (void)dev;      /* TODO */

    uint8_t size = iolist_size(iolist);

    /* Ignore send if packet size is 0 */
    if (!size) {
        return 0;
    }

    DEBUG("[ublox_g350] netdev: sending packet now (size: %d).\n", size);
    /* Write payload buffer */
    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
        if (iol->iol_len > 0) {
            /* write data to payload buffer */
        }
    }

    state = NETOPT_STATE_TX;
    netdev->driver->set(netdev, NETOPT_STATE, &state, sizeof(uint8_t));
    DEBUG("[ublox_g350] netdev: send: transmission in progress.\n");

    return 0;
}

static int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    DEBUG("[ublox_g350] netdev: read received data.\n");

    ublox_g350_t *dev = (ublox_g350_t *)netdev;
    uint8_t size = 0;
    (void)dev;      /* TODO */
    (void)info;     /* TODO */

    /* Get received packet info and size here */

    if (buf == NULL) {
        return size;
    }

    if (size > len) {
        return -ENOBUFS;
    }

    /* Read the received packet content here and write it to buf */

    return 0;
}

static int _init(netdev_t *netdev)
{
    ublox_g350_t *dev = (ublox_g350_t *)netdev;

    /* Launch initialization of driver and device */
    DEBUG("[ublox_g350] netdev: initializing driver...\n");
    if (ublox_g350_init(dev) != 0) {
        DEBUG("[ublox_g350] netdev: initialization failed\n");
        return -1;
    }

    DEBUG("[ublox_g350] netdev: initialization successful\n");
    return 0;
}

static void _isr(netdev_t *netdev)
{
    ublox_g350_t *dev = (ublox_g350_t *)netdev;
    (void)dev;      /* TODO */

    /* Handle IRQs here */
}

static int _get_state(ublox_g350_t *dev, void *val)
{
    (void)dev;      /* TODO */
    netopt_state_t state = NETOPT_STATE_OFF;
    memcpy(val, &state, sizeof(netopt_state_t));
    return sizeof(netopt_state_t);
}

static int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len)
{
    (void)max_len; /* unused when compiled without debug, assert empty */
    ublox_g350_t *dev = (ublox_g350_t *)netdev;
    (void)dev;      /* TODO */

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
    case NETOPT_STATE:
        assert(max_len >= sizeof(netopt_state_t));
        return _get_state(dev, val);

    default:
        break;
    }

    return -ENOTSUP;
}

static int _set_state(ublox_g350_t *dev, netopt_state_t state)
{
    (void)dev;      /* TODO */
    switch (state) {
    case NETOPT_STATE_STANDBY:
        DEBUG("[ublox_g350] netdev: set NETOPT_STATE_STANDBY state\n");
        break;

    case NETOPT_STATE_IDLE:
        DEBUG("[ublox_g350] netdev: set NETOPT_STATE_RX state\n");
        break;

    case NETOPT_STATE_RX:
        DEBUG("[ublox_g350] netdev: set NETOPT_STATE_RX state\n");
        break;

    case NETOPT_STATE_TX:
        DEBUG("[ublox_g350] netdev: set NETOPT_STATE_TX state\n");
        break;

    case NETOPT_STATE_RESET:
        DEBUG("[ublox_g350] netdev: set NETOPT_STATE_RESET state\n");
        break;

    default:
        return -ENOTSUP;
    }
    return sizeof(netopt_state_t);
}

static int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t len)
{
    (void)len; /* unused when compiled without debug, assert empty */
    ublox_g350_t *dev = (ublox_g350_t *)netdev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
    case NETOPT_STATE:
        assert(len == sizeof(netopt_state_t));
        return _set_state(dev, *((const netopt_state_t *)val));

    default:
        break;
    }

    return res;
}

const netdev_driver_t ublox_g350_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};
