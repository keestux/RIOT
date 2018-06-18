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
#include "xtimer.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

static int _eva8m_exists(eva8m_t* dev);

int eva8m_init(eva8m_t* dev, const eva8m_params_t* params)
{
    int result = 0;

    dev->params = *params;

    result = _eva8m_exists(dev);
    if (result < 0) {
        return result;
    }

    uint16_t avail;
    for (int ix = 0; ix < 10; ix++) {
        xtimer_usleep(50 * 1000u);
        result = eva8m_available(dev, &avail);
        if (result == 0) {
            break;
        }
    }

    return result;
}

static int _eva8m_exists(eva8m_t* dev)
{
    i2c_acquire(dev->params.i2c_dev);

    // i2c_write_byte(dev->params.i2c_dev, dev->params.i2c_addr, (uint8_t)command, 0);

    i2c_release(dev->params.i2c_dev);

    return 0;
}

int eva8m_available(eva8m_t* dev, uint16_t* avail)
{
    int result;
    uint8_t buffer[2];
    // TODO Check parms
    i2c_acquire(dev->params.i2c_dev);
    result = i2c_read_regs(dev->params.i2c_dev, dev->params.i2c_addr, 0xfd, buffer, sizeof(buffer), 0);
    i2c_release(dev->params.i2c_dev);
    DEBUG("[EVA8M] read regs: addr=0x%02x, result=%d\n", dev->params.i2c_addr, result);
    if (avail) {
        // 0xFD holds high byte, 0xFE holds low byte
        *avail = ((uint16_t)buffer[0] << 8) | buffer[1];
        DEBUG("[EVA8M] available=%u\n", *avail);
    }
    return result;
}

int eva8m_read_byte(eva8m_t* dev, uint8_t* b)
{
    int result;
    i2c_acquire(dev->params.i2c_dev);
    result = i2c_read_byte(dev->params.i2c_dev, dev->params.i2c_addr, b, 0);
    i2c_release(dev->params.i2c_dev);
    return result;
}
