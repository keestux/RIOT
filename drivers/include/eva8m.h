/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_eva8m u-blox EVA 8/8M
 * @ingroup     drivers_gps
 * @brief       Device driver interface for the u-blox EVA 8/8M series.
 *
 *
 * @{
 * @file
 * @brief       Device driver interface for the u-blox EVA 8/8M series.
 *
 * @details     The device communication is describe in
 *              "u-blox 8 / u-blox M8 Receiver Description" UBX-13003221
 *
 * @author      Kees Bakker <kees@sodaq.com>
 */

#ifndef EVA8M_H
#define EVA8M_H

#include <inttypes.h>
#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Parameters for the u-blox EVA 8/8M series.
 *
 * These parameters are needed to configure the device at startup.
 */
typedef struct {
    /* I2C details */
    i2c_t i2c_dev;                      /**< I2C device which is used */
    uint8_t i2c_addr;                   /**< I2C address */
} eva8m_params_t;

/**
 * @brief   Device descriptor for the EVA 8/8M
 */
typedef struct {
    eva8m_params_t params;               /**< Device Parameters */
} eva8m_t;

/**
 * @brief   Port Configuration
 */
typedef struct {
    uint8_t     portID;
    uint8_t     reserved1;
    uint16_t    txReady;
    uint32_t    mode;
    uint8_t     reserved2[4];
    uint16_t    inProtoMask;
    uint16_t    outProtoMask;
    uint16_t    flags;
    uint8_t     reserved3[2];
} eva8m_portconfig_t;

/**
 * @brief   Initialize the given EVA8M device
 *
 * @param[out] dev          Initialized device descriptor of EVA8M device
 * @param[in]  params       The parameters for the EVA8M device (sampling rate, etc)
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_init(eva8m_t* dev, const eva8m_params_t* params);

/**
 * @brief   Get number of available bytes in the given EVA8M device
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[out] avail        Pointer to the resulting number
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_available(eva8m_t* dev, uint16_t* avail);

/**
 * @brief   Read the next byte from the given EVA8M device's data stream
 *
 * In the common situation a data stream can be read by continuously
 * reading bytes. An explanation can be found in section "DDC Port" of
 * the above mention document.
 *
 * It depends on the output mode (NMEA protocal or the UBX protocol) what
 * the data stream looks like.
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[out] b            Pointer to the resulting byte
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_read_byte(eva8m_t* dev, uint8_t* b);

/**
 * @brief   Read the port config
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[out] portcfg      Pointer for the result
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_get_port_config(eva8m_t* dev, eva8m_portconfig_t* portcfg);

/**
 * @brief   Send UBX packet
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[in] msg_class     The message class
 * @param[in] msg_id        The message id
 * @param[in] payload       Pointer to the payload
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_send_ubx_packet(eva8m_t* dev, uint8_t msg_class, uint8_t msg_id, uint8_t* buffer, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif /* EVA8M_H */
/** @} */
