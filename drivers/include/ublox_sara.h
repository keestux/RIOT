/*
 * Copyright (C) 2024 Kees Bakker <kees@ijzerbout.nl>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ublox_sara Ublox GPRS driver
 * @ingroup     drivers_misc
 * @brief       Driver for Ublox GPRS modules
 *
 * This module provides functions to interact with Ublox GPRS modules.
 *
 * @{
 *
 * @file
 *
 * @author      Kees Bakker <kees@ijzerbout.nl>
 */

#ifndef UBLOX_SARA_H
#define UBLOX_SARA_H

#include <stdint.h>
#include <unistd.h>

#include "isrpipe.h"
#include "periph/uart.h"

#ifndef UBLOX_SARA_DEBUG
#define UBLOX_SARA_DEBUG (0)
#endif

#define UBLOX_SARA_OK       (0)
#define UBLOX_SARA_ERROR    (1)
#define UBLOX_SARA_CME_ERROR (2)
#define UBLOX_SARA_CMS_ERROR (3)
/* The following should not conflict with errno */
#define UBLOX_SARA_BAUDRATE_FAIL  (1097)
#define UBLOX_SARA_TIMEOUT  (1098)
#define UBLOX_SARA_UNKNOWN  (1099)

typedef struct ublox_sara_dev_s ublox_sara_dev_t;

/**
 * @brief   URC callback
 *
 * @param[in]   arg     optional argument
 * @param[in]   code    URC string received from the device
 */
typedef void (*ublox_sara_urc_cb_t)(ublox_sara_dev_t *dev, const char *str);

/**
 * @brief   Unsolicited result code data structure
 */
typedef struct {
    clist_node_t list_node; /**< node list */
    ublox_sara_urc_cb_t cb; /**< callback */
    const char *code;       /**< URC prefix string */
} ublox_sara_urc_t;

/**
 * @brief Ublox SARA device structure
 */
struct ublox_sara_dev_s {
    isrpipe_t isrpipe;      /**< isrpipe used for getting data from uart */
    uart_t uart;            /**< UART device where the device is attached */
    uint32_t baudrate;      /**< currently selected UART baud rate */
    bool echo_off;          /**< keep track echoing mode (was ATE0 sent?) */
    uint32_t cmd_timeout;   /**< the default timeout to wait for command OK */
    void (*switch_power)(ublox_sara_dev_t *, bool);  /**< function to switch power on/off */
    clist_node_t urc_list;  /**< list of registered URCs */
#if UBLOX_SARA_DEBUG
    bool at_start_of_line;
    bool need_in_out_marker;
#endif
};

/**
 * @brief   Initialize SARA device struct
 *
 * @param[in]   dev         device struct to initialize
 * @param[in]   uart        UART the device is connected to
 * @param[in]   baudrate    baudrate of the device
 * @param[in]   buf         input buffer
 * @param[in]   bufsize     size of @p buf
 *
 * @retval  0               Success
 * @retval  <0              On other errors from uart_init
 */
int ublox_sara_dev_init(ublox_sara_dev_t *dev, uart_t uart, uint32_t baudrate, char *buf, size_t bufsize);
void ublox_sara_power_on(ublox_sara_dev_t *dev);
void ublox_sara_power_off(ublox_sara_dev_t *dev);

/**
 * @brief   Is the device alive, does it react to AT?
 *
 * @param[in]   dev         device struct to initialize
 * @param[in]   retry_count the number of times to retry (0 means it will try it just once)
 *
 * @retval  true            the device is working, it responded to AT with OK
 * @retval  false           the device does not react to commands
*/
bool ublox_sara_is_alive(ublox_sara_dev_t *dev, size_t retry_count);

uint32_t ublox_sara_determine_baudrate(ublox_sara_dev_t *dev, uint32_t current_br, const uint32_t *rates);
int ublox_sara_change_baudrate(ublox_sara_dev_t *dev, uint32_t baudrate);

void ublox_sara_send_cmd(ublox_sara_dev_t *dev, const char *cmd);
int ublox_sara_send_cmd_wait_ok(ublox_sara_dev_t *dev,
                                const char *cmd, char *buffer, size_t buflen, uint32_t timeout);
void ublox_sara_send_str(ublox_sara_dev_t *dev, const char *str);
void ublox_sara_send_char(ublox_sara_dev_t *dev, char c);
int ublox_sara_read_resp(ublox_sara_dev_t *dev, char *buffer, size_t buflen, uint32_t timeout);
int ublox_sara_read_multi_resp(ublox_sara_dev_t *dev, char *buffer, size_t buflen, uint32_t timeout);
ssize_t ublox_sara_read_line(ublox_sara_dev_t *dev, char *buffer, size_t buflen, uint32_t timeout);
void ublox_sara_drain_rx(ublox_sara_dev_t *dev, uint32_t timeout);

void ublox_sara_process_urc(ublox_sara_dev_t *dev, const char *str);

extern const uint32_t ublox_sara_r4x_baudrates[];

#endif /* UBLOX_SARA_H */
/** @} */
