/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_bmx280
 * @{
 *
 * @file
 * @brief       Device driver for the u-blox EVA 8M series
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <string.h>

#include "board.h"
#include "eva8m.h"
#include "periph/i2c.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

static int _eva8m_exists(eva8m_t* dev);
static int _eva8m_available(eva8m_t* dev);

int eva8m_init(eva8m_t* dev, const eva8m_params_t* params)
{
    int result = EVA8M_OK;

    dev->params = *params;

    result = _eva8m_exists(dev);
    if (result < 0) {
        return result;
    }
    result = _eva8m_available(dev);

    return result;
}

static int _eva8m_exists(eva8m_t* dev)
{
    i2c_acquire(dev->params.i2c_dev);

    // i2c_write_byte(dev->params.i2c_dev, dev->params.i2c_addr, (uint8_t)command, 0);

    i2c_release(dev->params.i2c_dev);

    return EVA8M_OK;
}

static int _eva8m_available(eva8m_t* dev)
{
    int result;
    uint16_t nr_avail;
    i2c_acquire(dev->params.i2c_dev);
    result = i2c_read_regs(dev->params.i2c_dev, dev->params.i2c_addr, 0xfd, &nr_avail, sizeof(nr_avail), 0);
    i2c_release(dev->params.i2c_dev);
    DEBUG("[EVA8M] read regs: addr=0x%02x, result=%d\n", dev->params.i2c_addr, result);
    if (result < 0) {
        return -1;
    }
    DEBUG("[EVA8M] available=%u\n", nr_avail);
    return nr_avail;
}
