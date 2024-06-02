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
 *              The device periodically sends out info in one of two forms, a
 *              line of text terminated by <CR><LF>, or a UBX packet. The first
 *              should follow the NMEA protocol format. The latter has a UBlox
 *              specific format, called UBX. The device can also respond
 *              to commands, and then the response is always in UBX packets.
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
 * @brief   The size of the pre-allocated buffer
 *
 * The buffer is part of eva8m_t. It is meant as the first
 * place to store incoming packets. The size should be big enough
 * to receive NMEA protocol packets, and/or UBX packets.
 */
#define EVA8M_BUFFER_SIZE       256

/**
 * @brief   The default value for receive timeout (in milliseconds)
 */
#define EVA8M_DEFAULT_TIMEOUT   100

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

#define EVA8M_UBX_HEADER_BYTE1  0xB5
#define EVA8M_UBX_HEADER_BYTE2  0x62

/**
 * @brief   States for the receiving state machine
 */
typedef enum {
    EVA8M_RS_START,

    EVA8M_RS_SAW_DOLLAR,
    EVA8M_RS_SAW_CR,

    EVA8M_RS_SAW_HEADER_BYTE1,
    EVA8M_RS_SAW_HEADER,
    EVA8M_RS_SAW_LENGTH,
    EVA8M_RS_SAW_PAYLOAD,
    EVA8M_RS_SAW_CK_A,

    EVA8M_RS_SAW_END
} eva8m_receive_state_t;

/**
 * @brief   States for the receiving state machine
 */
typedef enum {
    EVA8M_PROT_UNKNOWN,
    EVA8M_PROT_NMEA,
    EVA8M_PROT_UBX,
} eva8m_protocol_t;

/**
 * @brief Enum for Class/ID
 *
 * The numbers are such that class is in the upper byte,
 * just like it is in the datasheet.
 */
typedef enum {
    UBX_NAV_PVT = 0x0107,
    UBX_NAV_SAT = 0x0135,
    UBX_ACK_NAK = 0x0500,
    UBX_ACK_ACK = 0x0501,
    UBX_CFG_PRT = 0x0600,
    UBX_CFG_MSG = 0x0601,
    UBX_CFG_TP5 = 0x0631,
    UBX_MON_VER = 0x0A04,
} eva8m_class_id_t;

/**
 * @brief   Device descriptor for the EVA 8/8M
 */
typedef struct {
    eva8m_params_t params;               /**< Device Parameters */
    uint8_t buffer[EVA8M_BUFFER_SIZE];
    uint8_t buffer_overflow;
    uint8_t checksum_error;
    eva8m_protocol_t prot;
    eva8m_receive_state_t state;
    uint16_t state_header_counter;
    uint16_t state_payload_length;
    uint16_t state_payload_counter;
    uint8_t received_ck_a;
    uint8_t received_ck_b;
    uint8_t computed_ck_a;
    uint8_t computed_ck_b;
    kernel_pid_t pps_thread_pid;
    uint32_t pps_counter;
    uint32_t btn0_counter;
} eva8m_t;

/**
 * @brief   Port Configuration (CFG-PRT)
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
 * @brief   NAV-PVT data structure
 */
typedef struct {
    uint32_t    iTOW;           /**<  0 GPS time of week of the navigation epoch. */
    uint16_t    year;           /**< 04 Year UTC */
    uint8_t     month;          /**< 06 Month, range 1..12 (UTC) */
    uint8_t     day;            /**< 07 Day of month, range 1..31 (UTC) */
    uint8_t     hour;           /**< 08 Hour of day, range 0..23 (UTC) */
    uint8_t     min;            /**< 09 Minute of hour, range 0..59 (UTC) */
    uint8_t     sec;            /**< 10 Seconds of minute, range 0..60 (UTC) */
    // uint8_t     valid;          /**< 11 Validity Flags */
    uint8_t     validDate:1;    /**< 11 Validity Flags */
    uint8_t     validTime:1;    /**< 11 Validity Flags */
    uint8_t     fullyResolved:1; /**< 11 Validity Flags */
    uint8_t     validMag:1;     /**< 11 Validity Flags */
    uint32_t    tAcc;           /**< 12 Time accuracy estimate (UTC) */
    int32_t     nano;           /**< 16 Fraction of second, range -1e9 .. 1e9 (UTC) */
    uint8_t     fixType;        /**< 20 GNSSfix Type: 0: no fix, 1: dead reckoning only, ... */
    uint8_t     flags;          /**< 21 Fix status flags */
    uint8_t     flags2;         /**< 22 Additional flags */
    uint8_t     numSV;          /**< 23 Number of satellites used in Nav Solution */
    int32_t     lon;            /**< 24 Longitude */
    int32_t     lat;            /**< 28 Latitude */
    int32_t     height;         /**< 32 Height above ellipsoid */
    int32_t     hMSL;           /**< 36 Height above mean sea level */
    uint32_t    hAcc;           /**< 40 Horizontal accuracy estimate */
    uint32_t    vAcc;           /**< 44 Vertical accuracy estimate */
    int32_t     velN;           /**< 48 NED north velocity */
    int32_t     velE;           /**< 52 NED east velocity */
    int32_t     velD;           /**< 56 NED down velocity */
    int32_t     gSpeed;         /**< 60 Ground Speed (2-D) */
    int32_t     headMot;        /**< 64 Heading of motion (2-D) */
    uint32_t    sAcc;           /**< 68 Speed accuracy estimate */
    uint32_t    headAcc;        /**< 72 Heading Accuracy Estimate (both motion and vehicle) */
    uint16_t    pDOP;           /**< 76 Position DOP */
    uint8_t     reserved1[6];   /**< 78 Reserved */
    int32_t     headVeh;        /**< 84 Heading of vehicle (2-D) */
    int16_t     magDec;         /**< 88 Magnetic declination */
    uint16_t    magAcc;         /**< 88 Magnetic declination accuracy */
} eva8m_nav_pvt_t;

/**
 * @brief   NAV-SAT data structure (head)
 */
typedef struct {
    uint32_t    iTOW;           /**<  0 GPS time of week of the navigation epoch. */
    uint8_t     version;        /**< 04 Message version (0x01 for this version) */
    uint8_t     numSvs;         /**< 05 Number of satellites */
    uint8_t     reserved1;      /**< 06 Reserved */
    /* The remainder is filled with repeated blocks, one per satellite */
} eva8m_nav_sat_head_t;

/**
 * @brief   NAV-SAT data structure (satellite)
 */
typedef struct {
    uint8_t     gnssId;         /**<  0 GNSS identifier (see Satellite Numbering) */
    uint8_t     svId;           /**< 01 Satellite identifier (see Satellite Numbering) */
    uint8_t     cno;            /**< 02 Carrier to noise ratio (signal strength) */
    int8_t      elev;           /**< 03 Elevation (range: +/-90), unknown if out of
                                        range */
    int16_t     azim;           /**< 04 Azimuth (range 0-360), unknown if
                                        elevation is out of range */
    int16_t     prRes;          /**< 06 Pseudorange residual */
    // uint32_t    flags;          /**< 08 Bitmask */
    uint32_t    qualityInd:3;   /**< Signal quality indicator: */
    uint32_t    svUsed:1;       /**< 1 = Signal in the subset specified in Signal
                                     Identifiers is currently being used for navigation */
    uint32_t    health:2;       /**< Signal health flag: */
    uint32_t    diffCorr:1;     /**< */
    uint32_t    smoothed:1;     /**< */
    uint32_t    orbitSource:3;  /**< */
    uint32_t    ephAvail:1;     /**< */
    uint32_t    almAvail:1;     /**< */
    uint32_t    anoAvail:1;     /**< */
    uint32_t    aopAvail:1;     /**< */
    uint32_t    sbasCorrUsed:1; /**< */
    uint32_t    rtcmCorrUsed:1; /**< */
    uint32_t    slasCorrUsed:1; /**< */
    uint32_t    spartnCorrUsed:1; /**< */
    uint32_t    prCorrUsed:1;   /**< */
    uint32_t    doCorrUsed:1;   /**< */
    uint32_t    clasCorrUsed:1; /**< */

    /* The remainder is filled with repeated blocks, one per satellite */
} eva8m_nav_sat_sv_t;

/**
 * @brief   Time Pulse Parameters (CFG-TP5)
 */
typedef struct {
    uint8_t     tpIdx;          /**< Time pulse selection (0=TIMEPULSE, 1=TIMEPULSE2) */
    uint8_t     version;        /**< Message version */
    uint8_t     reserved1[2];   /**< Reserved */
    int16_t     antCableDelay;  /**< Antenna cable delay */
    int16_t     rfGroupDelay;   /**< RF group delay */
    uint32_t    freqPeriod;     /**< Frequency or period time */
    uint32_t    freqPeriodLock; /**< Frequency or period time when locked to GPS time */
    uint32_t    pulseLenRatio;  /**< Pulse length or duty cycle */
    uint32_t    pulseLenRatioLock;  /**< Pulse length or duty cycle when locked to GPS time */
    int32_t     userConfigDelay;    /**< User configurable time pulse delay */
    uint32_t    flags;          /**< Configuration flags */
} eva8m_timepulseparm_t;

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
 * It depends on the output mode (NMEA protocol or the UBX protocol) what
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
 * @brief   Read the Time Pulse parameters
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[out] parm         Pointer for the result
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_get_timepulse_parm(eva8m_t* dev, eva8m_timepulseparm_t *parm);

/**
 * @brief   Send UBX packet
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[in] msg_class_id  The message class and id
 * @param[in] buffer        Pointer to the payload
 * @param[in] buflen        The size of the payload
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_send_ubx_packet(eva8m_t* dev, eva8m_class_id_t msg_class_id, uint8_t* buffer, size_t buflen);

/**
 * @brief   Receive UBX packet
 *
 * Use the buffer of @a dev to store the result.
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[in] timeout       Number of milliseconds timeout
 *
 * @return                  0 on success
 * @return                  I2C error code
 * @return                  -ETIMEDOUT no packet received in time
 */
int eva8m_receive_ubx_packet(eva8m_t* dev, uint16_t timeout);

/**
 * @brief   Send CFG-MSG
 *
 * @param[in] dev           The device descriptor of EVA8M device
 * @param[in] msg_class     The message class for the CFG-MSG
 * @param[in] msg_id        The message id for the CFG-MSG
 * @param[in] rate          The rate for the CFG-MSG
 *
 * @return                  0 on success
 * @return                  I2C error code
 */
int eva8m_send_cfg_msg(eva8m_t* dev, eva8m_class_id_t msg_class_id, uint8_t rate);

static inline eva8m_class_id_t eva8m_received_class_id(eva8m_t* dev) __attribute__((always_inline));
static inline eva8m_class_id_t eva8m_received_class_id(eva8m_t* dev)
{
    /* Assuming the received buffer contains B5,62,cls,id */
    return (eva8m_class_id_t)((uint16_t)(dev->buffer[2] << 8) | (uint16_t)dev->buffer[3]);
}

#ifdef __cplusplus
}
#endif

#endif /* EVA8M_H */
/** @} */
