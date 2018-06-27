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

#define ENABLE_DEBUG        (1)
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

int eva8m_get_port_config(eva8m_t* dev, eva8m_portconfig_t* portcfg)
{
    int result;

    DEBUG("[EVA8M] eva8m_get_port_config\n");

    memset(portcfg, 0, sizeof(*portcfg));

    i2c_acquire(dev->params.i2c_dev);

    /* Send the command to get the port config */
    result = eva8m_send_ubx_packet(dev, 0x06, 0x00, NULL, 0);

    /* Receive the requested config */
    if (result == 0) {
        // TODO
        // result = eva8m_receive_ubx_packet(dev, ...);
    }

    i2c_release(dev->params.i2c_dev);

    return result;
}

static inline void _update_checksum(uint8_t* ck_a, uint8_t* ck_b, uint8_t b) __attribute__((always_inline));
static inline void _update_checksum(uint8_t* ck_a, uint8_t* ck_b, uint8_t b)
{
    *ck_a += b;
    *ck_b += *ck_a;
}

int eva8m_send_ubx_packet(eva8m_t* dev, uint8_t msg_class, uint8_t msg_id, uint8_t* buffer, size_t buflen)
{
    int result;
    uint8_t header[2] = { 0xb5, 0x62 };
    uint8_t class_id_len[4];
    uint8_t ck_a;
    uint8_t ck_b;

    DEBUG("[EVA8M] eva8m_send_ubx_packet\n");

    class_id_len[0] = msg_class;
    class_id_len[1] = msg_id;
    if (buffer) {
        class_id_len[2] = buflen & 0xFF;
        class_id_len[3] = (buflen >> 8) & 0xFF;
    }
    else {
        class_id_len[2] = 0;
        class_id_len[3] = 0;
    }

    /* "The checksum is calculated over the Message, starting and
     * including the CLASS field, up until, but excluding, the
     * Checksum Field"
     */
    ck_a = 0;
    ck_b = 0;
    for (size_t i = 0; i < sizeof(class_id_len); i++) {
        _update_checksum(&ck_a, &ck_b, class_id_len[i]);
    }
    if (buffer) {
        for (size_t i = 0; i < buflen; i++) {
            _update_checksum(&ck_a, &ck_b, buffer[i]);
        }
    }

    result = i2c_write_bytes(dev->params.i2c_dev, dev->params.i2c_addr, header, sizeof(header), I2C_NOSTOP);
    if (result == 0) {
        result = i2c_write_bytes(dev->params.i2c_dev, dev->params.i2c_addr, class_id_len, sizeof(class_id_len), I2C_NOSTART | I2C_NOSTOP);
    }
    if (buffer) {
        if (result == 0) {
            result = i2c_write_bytes(dev->params.i2c_dev, dev->params.i2c_addr, buffer, buflen, I2C_NOSTART | I2C_NOSTOP);
        }
    }
    if (result == 0) {
        result = i2c_write_byte(dev->params.i2c_dev, dev->params.i2c_addr, ck_a, I2C_NOSTART | I2C_NOSTOP);
    }
    if (result == 0) {
        result = i2c_write_byte(dev->params.i2c_dev, dev->params.i2c_addr, ck_b, I2C_NOSTART);
    }

    return result;
}
