/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_eva8m
 * @{
 *
 * @file
 * @brief       Device driver for the u-blox EVA 8M series
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <errno.h>
#include <string.h>

#include "board.h"
#include "eva8m.h"
#include "periph/i2c.h"
#include "ztimer.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

static inline bool is_timedout(uint32_t from, uint32_t nr_ms) __attribute__((always_inline));
static inline bool is_timedout(uint32_t from, uint32_t nr_ms)
{
    return (ztimer_now(ZTIMER_MSEC) - from) > nr_ms;
}

int eva8m_init(eva8m_t* dev, const eva8m_params_t* params)
{
    int result = 0;

    dev->params = *params;
    dev->pps_thread_pid = thread_getpid();

    uint16_t avail;
    for (int ix = 0; ix < 10; ix++) {
        ztimer_sleep(ZTIMER_MSEC, 50);
        result = eva8m_available(dev, &avail);
        if (result == 0) {
            break;
        }
    }

    return result;
}

int eva8m_available(eva8m_t* dev, uint16_t* avail)
{
    int result;
    uint8_t buffer[2];
    // TODO Check parms
    i2c_acquire(dev->params.i2c_dev);
    result = i2c_read_regs(dev->params.i2c_dev, dev->params.i2c_addr, 0xfd, buffer, sizeof(buffer), 0);
    i2c_release(dev->params.i2c_dev);
    // DEBUG("[EVA8M] read regs: addr=0x%02x, result=%d\n", dev->params.i2c_addr, result);
    if (avail) {
        // 0xFD holds high byte, 0xFE holds low byte
        *avail = ((uint16_t)buffer[0] << 8) | buffer[1];
        // DEBUG("[EVA8M] available=%u\n", *avail);
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

static int _eva8m_get_data(eva8m_t* dev,
                           eva8m_class_id_t msg_class_id,
                           uint8_t *buffer, size_t buflen)
{
    int result;

    memset(buffer, 0, buflen);

    /* Send the command to get the data */
    result = eva8m_send_ubx_packet(dev, msg_class_id, NULL, 0);
    DEBUG("[EVA8M] send packet (sg_class_id=0x%04X, result=%d)\n",
          (unsigned)msg_class_id, result);
    /* Receive the requested data */
    if (result == 0) {
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        DEBUG("[EVA8M] receive packet (sg_class_id=0x%04X, result=%d)\n",
              (unsigned)eva8m_received_class_id(dev), result);
        if (result == 0 && eva8m_received_class_id(dev) == msg_class_id) {
            DEBUG("[EVA8M] received packet\n");
            memcpy(buffer, &dev->buffer[6], buflen);
        } else {
            /* Else what? Ignore? */
        }
        /* ACK / NACK */
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        if (result == 0 && eva8m_received_class_id(dev) == UBX_ACK_ACK) {
            // DEBUG("[EVA8M] received ACK\n");
        }
        else if (eva8m_received_class_id(dev) == UBX_ACK_NAK) {
            DEBUG("[EVA8M] received NACK\n");
        }
    }

    return result;
}

int eva8m_get_port_config(eva8m_t* dev, eva8m_portconfig_t* portcfg)
{
    int result;

    DEBUG("[EVA8M] eva8m_get_port_config\n");

    memset(portcfg, 0, sizeof(*portcfg));

    /* Send the command to get the port config */
    result = eva8m_send_ubx_packet(dev, UBX_CFG_PRT, NULL, 0);

    /* Receive the requested config */
    if (result == 0) {
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        if (result == 0 && eva8m_received_class_id(dev) == UBX_CFG_PRT) {
            // DEBUG("[EVA8M] received port_config\n");
            // DEBUG("[EVA8M] %02x %02x %02x %02x\n", dev->buffer[2], dev->buffer[3], dev->buffer[4], dev->buffer[5]);
            memcpy(portcfg, &dev->buffer[6], sizeof(*portcfg));
            // DEBUG("[EVA8M]  inProtoMask %04X\n", portcfg->inProtoMask);
            // DEBUG("[EVA8M] outProtoMask %04X\n", portcfg->outProtoMask);
        }
        /* ACK / NACK */
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        if (result == 0 && eva8m_received_class_id(dev) == UBX_ACK_ACK) {
            // DEBUG("[EVA8M] received ACK\n");
        }
        else if (eva8m_received_class_id(dev) == UBX_ACK_NAK) {
            DEBUG("[EVA8M] received NACK\n");
        }
    }

    return result;
}

int eva8m_get_timepulse_parm(eva8m_t* dev, eva8m_timepulseparm_t *parm)
{
    DEBUG("[EVA8M] eva8m_get_timepulse_parm\n");

    return _eva8m_get_data(dev, UBX_CFG_TP5, (uint8_t*)parm, sizeof(*parm));
}

static inline void _update_checksum(uint8_t* ck_a, uint8_t* ck_b, uint8_t b) __attribute__((always_inline));
static inline void _update_checksum(uint8_t* ck_a, uint8_t* ck_b, uint8_t b)
{
    *ck_a += b;
    *ck_b += *ck_a;
}

int eva8m_send_ubx_packet(eva8m_t* dev,
                          eva8m_class_id_t msg_class_id,
                          uint8_t* buffer, size_t buflen)
{
    int result;
    uint8_t header[2] = { 0xb5, 0x62 };
    uint8_t class_id_len[4];
    uint8_t ck_a;
    uint8_t ck_b;

    DEBUG("[EVA8M] eva8m_send_ubx_packet\n");

    class_id_len[0] = (uint8_t)(msg_class_id >> 8);
    class_id_len[1] = (uint8_t)(msg_class_id & 0xFF);
    if (buffer && buflen > 0) {
        /* Length in little endian */
        class_id_len[2] = buflen & 0xFF;
        class_id_len[3] = (buflen >> 8) & 0xFF;
    }
    else {
        /* No buffer, this automativally identicates a GET */
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

    i2c_acquire(dev->params.i2c_dev);

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

    i2c_release(dev->params.i2c_dev);

    return result;
}

/**
 * @brief   Update the EVA8M receiver state machine
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[in] b             The next received byte
 */
static void _eva8m_receive_ubx_sm_update(eva8m_t* dev, uint8_t b)
{
    switch (dev->state) {
    case EVA8M_RS_START:
        if (b == EVA8M_UBX_HEADER_BYTE1) {
            dev->prot = EVA8M_PROT_UBX;
            dev->state = EVA8M_RS_SAW_HEADER_BYTE1;
        }
        else if ((char)b == '$') {
            dev->prot = EVA8M_PROT_NMEA;
            dev->state = EVA8M_RS_SAW_DOLLAR;
        }
        else {
            /* No state change */
        }
        break;

    case EVA8M_RS_SAW_DOLLAR:
        /* Collect line until <CR> is seen */
        if ((char)b == '\r') {
            dev->state = EVA8M_RS_SAW_CR;
        }
        else {
            /* Normal character, part of the NMEA line */
            /* TODO Compute checksum */
        }
        break;
    case EVA8M_RS_SAW_CR:
        if ((char)b == '\n') {
            dev->state = EVA8M_RS_SAW_END;
        }
        else {
            /* There should have been a <LF> after the <CR> */
            dev->prot = EVA8M_PROT_UNKNOWN;
            dev->state = EVA8M_RS_START;
        }
        break;

    case EVA8M_RS_SAW_HEADER_BYTE1:
        if (b == EVA8M_UBX_HEADER_BYTE2) {
            dev->state = EVA8M_RS_SAW_HEADER;
            dev->state_header_counter = 0;
        }
        else {
            dev->prot = EVA8M_PROT_UNKNOWN;
            dev->state = EVA8M_RS_START;
        }
        break;
    case EVA8M_RS_SAW_HEADER:
        _update_checksum(&dev->computed_ck_a, &dev->computed_ck_b, b);
        dev->state_header_counter++;
        switch (dev->state_header_counter) {
        case 1:
            /* class */
            break;
        case 2:
            /* id */
            break;
        case 3:
            /* LS byte of length */
            dev->state_payload_length = b;
            break;
        case 4:
            /* MS byte of length */
            /* FALL THROUGH */
        default:
            dev->state_payload_length |= (b >> 8);
            dev->state = EVA8M_RS_SAW_LENGTH;
            dev->state_payload_counter = 0;
            break;
        }
        break;
    case EVA8M_RS_SAW_LENGTH:
        _update_checksum(&dev->computed_ck_a, &dev->computed_ck_b, b);
        dev->state_payload_counter++;
        if (dev->state_payload_counter >= dev->state_payload_length) {
            dev->state = EVA8M_RS_SAW_PAYLOAD;
            dev->state_payload_counter = 0;
        }
        break;

    case EVA8M_RS_SAW_PAYLOAD:
        /* ck_a */
        dev->received_ck_a = b;
        dev->state = EVA8M_RS_SAW_CK_A;
        break;
    case EVA8M_RS_SAW_CK_A:
        /* ck_b */
        dev->received_ck_b = b;
        dev->state = EVA8M_RS_SAW_END;
        if ((dev->received_ck_a == dev->computed_ck_a)
            && (dev->received_ck_b == dev->computed_ck_b)) {
            // DEBUG("[EVA8M] checksum OK\n");
        }
        else {
            DEBUG("[EVA8M] checksum error, rcvd=0x%02X,0x%02X comp=0x%02X,0x%02X\n",
                  dev->received_ck_a, dev->received_ck_b,
                  dev->computed_ck_a, dev->computed_ck_b);
            dev->checksum_error = 1;
        }
        break;

    case EVA8M_RS_SAW_END:
        /* This is not expected, but harmless. */
        break;
    }
}

/**
 * @brief   Reset the state machine
 *
 * @param[out] dev          Initialized device descriptor of EVA8M device
 */
static void _eva8m_reset_sm(eva8m_t* dev)
{
    dev->state = EVA8M_RS_START;
    memset(dev->buffer, 0, sizeof(dev->buffer));
    dev->buffer_overflow = 0;
    dev->checksum_error = 0;
    dev->prot = EVA8M_PROT_UNKNOWN;
    dev->computed_ck_a = 0;
    dev->computed_ck_b = 0;
}

int eva8m_receive_ubx_packet(eva8m_t* dev, uint16_t timeout)
{
    // DEBUG("[EVA8M] eva8m_receive_ubx_packet\n");

    int result = 0;
    size_t buf_ix = 0;

    _eva8m_reset_sm(dev);

    uint32_t start_time = ztimer_now(ZTIMER_MSEC);
    while (dev->state != EVA8M_RS_SAW_END &&
            !is_timedout(start_time, timeout)) {
        uint16_t nr_avail;
        result = eva8m_available(dev, &nr_avail);
        if (result == 0) {
            for (uint16_t ix = 0; dev->state != EVA8M_RS_SAW_END && ix < nr_avail; ix++) {
                uint8_t b;
                result = eva8m_read_byte(dev, &b);
                if (result == 0) {
                    _eva8m_receive_ubx_sm_update(dev, b);
                    /* Leave room for the string terminator */
                    if (buf_ix < (sizeof(dev->buffer) - 1)) {
                        if (dev->state != EVA8M_RS_START) {
                            dev->buffer[buf_ix++] = b;
                        }
                    }
                    else {
                        /* Buffer overflow */
                        dev->buffer_overflow = 1;
                    }
                }
            }
        }
    }
    if (dev->state != EVA8M_RS_SAW_END) {
        /* Timed out */
        result = -ETIMEDOUT;
    }
    return result;
}

int eva8m_send_cfg_msg(eva8m_t* dev, eva8m_class_id_t msg_class_id, uint8_t rate)
{
    uint8_t buffer[3];

    buffer[0] = (msg_class_id >> 8) & 0xff;
    buffer[1] = msg_class_id & 0xff;
    buffer[2] = rate;

    return eva8m_send_ubx_packet(dev, UBX_CFG_MSG, buffer, sizeof(buffer));
}
