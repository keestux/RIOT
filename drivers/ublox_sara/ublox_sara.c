/*
 * Copyright (C) 2024 Kees Bakker <kees@ijzerbout.nl>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ublox_sara.h"

#include "board.h"
#include "periph/gpio.h"
#include "isrpipe.h"
#include "isrpipe/read_timeout.h"
#include "periph/uart.h"
#include "timex.h"
#include "ztimer.h"
#include "ztimer/stopwatch.h"

/**
 * @brief Command line termination character S3
 */
#ifndef UBLOX_SARA_S3
#define UBLOX_SARA_S3   '\r'
#endif

#define MY_MSG_TYPE_TIMEOUT     (0x6789)

#define ENABLE_DEBUG        0
#include "debug.h"

/**
 * @brief       R4X supported baudrates
 *
 * Documentation is a bit confusing.
 * SARA-R404M / SARA-R410M-01B - 9600, 19200, 38400, 57600, 115200 (default and
 * factory-programmed value)
 *  • SARA-R410M-02B / SARA-R410M-52B / SARA-R410M-63B / SARA-R410M-73B /
 *    SARA-R410M-83B / SARA-R412M - The information text response to the test
 *    command returns a list of baud rates; within this list the only supported baud rates
 *    are: 9600, 19200, 38400, 57600, 115200 (default and factory-programmed value),
 *    230400, 460800.
 *  • SARA-R422-00B / SARA-R422M8S / SARA-R422S-00B - 9600, 19200, 38400, 57600,
 *    115200 (default and factory-programmed value), 230400, 460800
 *  • SARA-R422-01B / SARA-R422M10S / SARA-R422S-01B / LEXI-R4 - 9600, 19200,
 *    38400, 57600, 115200 (default and factory-programmed value), 230400, 460800,
 *    921600
 *
 * --> On SARA-R410M-02B-00 the information text response to the test command
 *     returns a list of baud rates; within this list the only supported baud rates are:
 *     9600, 19200, 38400, 57600, 115200 (default and factory-programmed value).
 */
const uint32_t ublox_sara_r4x_baudrates[] = {
    115200,
    230400,     /* not listed in the special SARA-R410M-02B-00 note, but works */
    460800,     /* not listed in the special SARA-R410M-02B-00 note, but works */
    9600,
    19200,
    38400,
    57600,
    0,          /* terminates the list */
};

/**
 * @brief   Unsolicited result code data structure
 */
typedef struct {
    ublox_sara_dev_t *dev;
    const char *str;
} ublox_sara_urc_handler_arg_t;

static void _handle_creg_urc(ublox_sara_dev_t *dev, const char *str);
static ublox_sara_urc_t _urc_list[] = {
    {
        .code = "+CREG:",
        .cb = _handle_creg_urc,
    },
};

#if UBLOX_SARA_DEBUG
static void _debug_print_char(ublox_sara_dev_t *dev, char c, bool in_out)
{
    static int prev_in_out = 2;
    if ((prev_in_out == 1 && in_out) || (prev_in_out == 0 && !in_out)) {
        /* No change of in_out */
    }
    else {
        prev_in_out = in_out ? 1 : 0;
        if (!dev->at_start_of_line) {
            printf("\n");
            dev->at_start_of_line = true;
        }
        dev->need_in_out_marker = true;
    }

    if (c == '\n') {
        printf("\\n\n");
        dev->at_start_of_line = true;
        dev->need_in_out_marker = true;
    }
    else {
        if (dev->at_start_of_line) {
            if (dev->need_in_out_marker) {
                if (in_out) {
                    printf("<< ");
                }
                else {
                    printf(">> ");
                }
                dev->need_in_out_marker = false;
            }
        }
        if (c == '\r') {
            printf("\\r");
        }
        else if (isprint(c)) {
            putchar(c);
        }
        else {
            printf("\\x%02x", c);
        }
        dev->at_start_of_line = false;
    }
}
#else
static void _debug_print_char(ublox_sara_dev_t *dev, char c, bool in_out)
{
    (void)dev;
    (void)c;
    (void)in_out;
}
#endif

#if UBLOX_SARA_DEBUG
static void _debug_print_chars(ublox_sara_dev_t *dev, const char *str, bool in_out)
{
    while (*str) {
        _debug_print_char(dev, *str++, in_out);
    }
}
#else
static void _debug_print_chars(ublox_sara_dev_t *dev, const char *str, bool in_out)
{
    (void)dev;
    (void)str;
    (void)in_out;
}
#endif
#if UBLOX_SARA_DEBUG
static void _debug_print_reset(ublox_sara_dev_t *dev)
{
    printf("\n");
    dev->at_start_of_line = true;
    dev->need_in_out_marker = true;
}
#else
static void _debug_print_reset(ublox_sara_dev_t *dev)
{
    (void)dev;
}
#endif

static bool _startswith(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

/**
 * @brief   Write one byte in the receive buffer
 *
 * @param[in]   dev         device to operate on
 * @param[in]   data        the byte that was just received in the RX ISR
 */
static void _isrpipe_rx_write_one(void *_dev, uint8_t data)
{
    ublox_sara_dev_t *dev = (ublox_sara_dev_t *) _dev;
    isrpipe_write_one(&dev->isrpipe, data);
}

/**
 * @brief   Switch power of the Ublox SARA device on or off
 *
 * @param[in]   dev         device struct
 * @param[in]   on_off      true if switching on, false if switching off
 */
void _switch_power(ublox_sara_dev_t *dev, bool on_off)
{
    (void)dev;

    if (on_off) {
#ifdef SARA_ENABLE_ON
        SARA_ENABLE_ON;
#endif
#ifdef SARA_TX_ENABLE_ON
        SARA_TX_ENABLE_ON;
#endif
#ifdef SARA_STATUS_PIN
        // printf("SARA_STATUS: %d\n", gpio_read(SARA_STATUS_PIN));
#endif

#ifdef SARA_R4XX_PWR_ON_PIN
        gpio_init(SARA_R4XX_PWR_ON_PIN, GPIO_OUT);
        SARA_R4XX_PWR_ON_OFF;
        ztimer_sleep(ZTIMER_MSEC, 2000);
        SARA_R4XX_PWR_ON_ON;
        gpio_init(SARA_R4XX_PWR_ON_PIN, GPIO_IN);
#endif
    }
    else {
#ifdef SARA_ENABLE_OFF
        SARA_ENABLE_OFF;
#endif
#ifdef SARA_TX_ENABLE_OFF
        SARA_TX_ENABLE_OFF;
#endif
    }
#ifdef SARA_STATUS_PIN
    // printf("SARA_STATUS: %d\n", gpio_read(SARA_STATUS_PIN));
#endif
}

void ublox_sara_power_on(ublox_sara_dev_t *dev)
{
    if (dev->switch_power) {
        (*dev->switch_power)(dev, true);
    }
    // (void)uart_init(dev->uart, dev->baudrate, _isrpipe_rx_write_one, dev);
    uart_poweron(dev->uart);
}

void ublox_sara_power_off(ublox_sara_dev_t *dev)
{
    if (dev->switch_power) {
        (*dev->switch_power)(dev, false);
    }
    uart_poweroff(dev->uart);
}

int ublox_sara_dev_init(ublox_sara_dev_t *dev, uart_t uart, uint32_t baudrate, char *buf, size_t bufsize)
{
    dev->uart = uart;
    dev->echo_off = false;
    dev->cmd_timeout = 1000 * US_PER_MS;
    dev->switch_power = _switch_power;
#if UBLOX_SARA_DEBUG
    dev->at_start_of_line = true;
    dev->need_in_out_marker = true;
#endif

    dev->urc_list.next = NULL;
    for (size_t ix = 0; ix < ARRAY_SIZE(_urc_list); ix++) {
        clist_rpush(&dev->urc_list, &(_urc_list[ix].list_node));
    }

    isrpipe_init(&dev->isrpipe, (uint8_t *)buf, bufsize);

    dev->baudrate = baudrate;
    int res = uart_init(dev->uart, baudrate, _isrpipe_rx_write_one, dev);

    /* Start in power off state */
    // ublox_sara_power_off(dev);

    return res;
}

void ublox_sara_register_power_func(ublox_sara_dev_t *dev, void (*func)(ublox_sara_dev_t *, bool))
{
    dev->switch_power = func;
}

bool ublox_sara_is_alive(ublox_sara_dev_t *dev, size_t retry_count)
{
    char buffer[10];        /**< buffer to read "OK" */
    bool res = false;
    do {
        if (ublox_sara_send_cmd_wait_ok(dev, "AT", buffer, sizeof(buffer), 450 * US_PER_MS) == UBLOX_SARA_OK) {
            res = true;
            break;
        }
        if (retry_count > 0) {
            retry_count--;
        }
    } while (retry_count > 0);
    return res;
}

uint32_t ublox_sara_determine_baudrate(ublox_sara_dev_t *dev, uint32_t current_br, const uint32_t *rates)
{
    uint32_t baudrate = current_br;
    const uint8_t retry_count = 5;

    /* Try current baudrate first
    */
    if (baudrate != 0) {
        printf("Trying current baudrate: %" PRIu32 "\n", baudrate);
        if (ublox_sara_is_alive(dev, retry_count)) {
            return baudrate;
        }
    }
    for (size_t ix = 0; ; ix++) {
        baudrate = rates[ix];
        if (baudrate == 0) {
            break;
        }
        printf("Trying baudrate: %" PRIu32 "\n", baudrate);
        dev->baudrate = baudrate;
        (void)uart_init(dev->uart, baudrate, _isrpipe_rx_write_one, dev);
        if (ublox_sara_is_alive(dev, retry_count)) {
            return baudrate;
        }
    }

    return baudrate;
}

int ublox_sara_change_baudrate(ublox_sara_dev_t *dev, uint32_t baudrate)
{
    char cmd[20];
    char response[50];
    snprintf(cmd, sizeof(cmd), "AT+IPR=%" PRIu32, baudrate);
    int res;
    res = ublox_sara_send_cmd_wait_ok(dev, cmd, response, sizeof(response), dev->cmd_timeout);
    if (res == UBLOX_SARA_OK) {
        /* On the UART AT interface, after the reception of the "OK" result code for
        * the +IPR command, the DTE shall wait for at least 100 ms before issuing a
        * new AT command; this is to guarantee a proper baud ratereconfiguration.
        */
        ztimer_sleep(ZTIMER_USEC, 150 * US_PER_MS);
        dev->baudrate = baudrate;
        (void)uart_init(dev->uart, baudrate, _isrpipe_rx_write_one, dev);
        if (!ublox_sara_is_alive(dev, 5)) {
            res = UBLOX_SARA_BAUDRATE_FAIL;
        }
    }
    return res;
}

static int _read_echoed_char(ublox_sara_dev_t *dev, char c)
{
    int res = UBLOX_SARA_OK;

    if (!dev->echo_off) {
        char c2;
        int read_res;
        read_res = isrpipe_read_timeout(&dev->isrpipe, (uint8_t *)&c2, 1, 100 * US_PER_MS);
        if (read_res == 1) {
            _debug_print_char(dev, c2, true);
            (void)c;
            /* TODO Should we check c == c2 ? And if we do, what if they don't match? */
            // if (c2 == UBLOX_SARA_S3) {
            //     /* Special case. After this we need to start a new line at debug output */
            // }
        }
        else if (read_res == -ETIMEDOUT) {
            res = UBLOX_SARA_TIMEOUT;
        }
        else {
            res = UBLOX_SARA_UNKNOWN;
        }
    }
    return res;
}

static int _read_echoed_chars(ublox_sara_dev_t *dev, const char *cmd)
{
    int res = UBLOX_SARA_OK;
    while (res == UBLOX_SARA_OK && *cmd) {
        res = _read_echoed_char(dev, *cmd++);
    }
    return res;
}

void ublox_sara_send_cmd(ublox_sara_dev_t *dev, const char *cmd)
{
    ublox_sara_drain_rx(dev, 100 * US_PER_MS);
    // ublox_sara_send_str(dev, "AT");
    ublox_sara_send_str(dev, cmd);
    ublox_sara_send_char(dev, UBLOX_SARA_S3);
    int res = UBLOX_SARA_OK;
    if (res == UBLOX_SARA_OK) {
        res = _read_echoed_chars(dev, cmd);
    }
    if (res == UBLOX_SARA_OK) {
        res = _read_echoed_char(dev, UBLOX_SARA_S3);
    }
    if (!dev->echo_off && res != UBLOX_SARA_OK) {
        _debug_print_reset(dev);
    }
}

int ublox_sara_send_cmd_wait_ok(ublox_sara_dev_t *dev, const char *cmd, char *buffer, size_t buflen, uint32_t timeout)
{
    int res = UBLOX_SARA_UNKNOWN;

    ublox_sara_send_cmd(dev, cmd);
    res = ublox_sara_read_resp(dev, buffer, buflen, timeout);

    return res;
}

int ublox_sara_read_resp(ublox_sara_dev_t *dev, char *buffer, size_t buflen, uint32_t timeout)
{
    int res = UBLOX_SARA_UNKNOWN;
    char *ptr = buffer;
    size_t remaining_len = buflen;
    size_t nr_lines = 0;            /**< keep track of received lines */
    uint32_t stopwatch_count;
    ztimer_stopwatch_t stopwatch;    /**< keep track of timing out read_resp */

    ztimer_stopwatch_init(ZTIMER_USEC, &stopwatch);
    ztimer_stopwatch_start(&stopwatch);

    while ((stopwatch_count = ztimer_stopwatch_measure(&stopwatch)) < timeout) {
        ssize_t resp_len;
        resp_len = ublox_sara_read_line(dev, ptr, remaining_len, timeout - stopwatch_count);
        if (resp_len == 0 ||
            strcmp(ptr, " \r") == 0 ||
            strcmp(ptr, " \r\n") == 0 ||
            strcmp(ptr, "\r\n") == 0 ||
            strcmp(ptr, "\r\r\n") == 0) {
            /* Ignore empty lines */
            continue;
        }
        if (resp_len < 0) {
            res = resp_len;
            // break;
            /* Keep on trying? */
            continue;
        }
        nr_lines++;
        if (_startswith(ptr, "OK\r\n")) {
            *ptr = '\0';        /* Scratch the OK\r\n */
            nr_lines--;
            if (nr_lines == 1) {
                /* Strip the line ending (CRLF) */
                if ((ptr - buffer) >=2) {
                    ptr -= 2;
                    *ptr = '\0';
                }
            }
            res = UBLOX_SARA_OK;
            break;
        }
        else if (_startswith(ptr, "ERROR\r\n")) {
            res = UBLOX_SARA_ERROR;
            break;
        }
        else if (_startswith(ptr, "+CME ERROR:")) {
            res = UBLOX_SARA_CME_ERROR;
            break;
        }
        else if (_startswith(ptr, "+CMS ERROR:")) {
            res = UBLOX_SARA_CMS_ERROR;
            break;
        }
        ublox_sara_process_urc(dev, ptr);
        ptr += resp_len;
        remaining_len -= resp_len;
    }
    if (stopwatch_count >= timeout) {
        res = -ETIMEDOUT;
        // printf("stopwatch_count=%ld\n", stopwatch_count);
    }
    ztimer_stopwatch_stop(&stopwatch);

    // printf("ublox_sara_read_resp: res=%d, nr_lines=%d, >>%s<<\n", res, nr_lines, buffer);
    return res;
}

ssize_t ublox_sara_read_line(ublox_sara_dev_t *dev, char *buffer, size_t buflen, uint32_t timeout)
{
    ssize_t res = 0;
    char *ptr = buffer;
    uint32_t stopwatch_count;
    ztimer_stopwatch_t stopwatch;   /**< keep track of timing out read_resp */
    char prev_rx_c = '\0';          /**< previous received char */

    ztimer_stopwatch_init(ZTIMER_USEC, &stopwatch);
    ztimer_stopwatch_start(&stopwatch);

    while ((stopwatch_count = ztimer_stopwatch_measure(&stopwatch)) < timeout) {
        int read_res;
        char c;
        if (prev_rx_c == '\r') {
            /* Wait 1ms to give the device a chance to send the next char
             * In case of the CRLF sequence (normal case) it will be soon enough.
             */
            ztimer_sleep(ZTIMER_USEC, 5 * US_PER_MS);
            read_res = tsrb_peek(&dev->isrpipe.tsrb, (uint8_t *)&c, 1);
            if (read_res == 1) {
                if (c != '\n') {
                    /* Sometimes the device sends a CR not followed by LF.
                     * In that case return the result as a separate line.
                     * Add a fake LF.
                     */
                    _debug_print_char(dev, '\n', true);
                    *ptr++ = '\n';
                    buflen--;
                    res++;
                    break;
                }
            }
        }
        read_res = isrpipe_read_timeout(&dev->isrpipe, (uint8_t *)&c, 1, timeout - stopwatch_count);
        if (read_res == 1) {
            _debug_print_char(dev, c, true);
            *ptr++ = c;
            buflen--;
            res++;
            prev_rx_c = c;
            if (c == '\n') {
                break;
            }
        }
        else if (read_res == -ETIMEDOUT) {
            /* We timed out. What do we do with the buffer contents so far? */
            res = read_res;
            break;
        }
        else {
            /* Unknown error */
            res = -UBLOX_SARA_UNKNOWN;
            break;
        }
    }
    *ptr = '\0';
    if (stopwatch_count >= timeout) {
        res = -ETIMEDOUT;
        // printf("stopwatch_count=%ld\n", stopwatch_count);
    }
    ztimer_stopwatch_stop(&stopwatch);

    // if (res < 0) {
    //     printf("ublox_sara_read_line: res=%d\n", res);
    // }
    // else {
    //     printf("ublox_sara_read_line: res=%d, >>%s<<\n", res, buffer);
    // }
    return res;
}

void ublox_sara_drain_rx(ublox_sara_dev_t *dev, uint32_t timeout)
{
    /* TODO We need a better safety precaution to avoid endless loop. Better than "while (1) {}" */
    for (size_t ix = 0; ix < 10000; ix++) {
        int read_res;
        char c;
        read_res = isrpipe_read_timeout(&dev->isrpipe, (uint8_t *)&c, 1, timeout);
        if (read_res == 1) {
            _debug_print_char(dev, c, true);
        }
        else {
            /* Probably a timeout */
            break;
        }
    }
}

static int _check_urc(clist_node_t *node, void *arg)
{
    ublox_sara_urc_handler_arg_t *handler_arg = arg;
    ublox_sara_urc_t *urc = container_of(node, ublox_sara_urc_t, list_node);

    DEBUG("Trying to match with %s\n", urc->code);

    if (_startswith(handler_arg->str, urc->code)) {
        urc->cb(handler_arg->dev, handler_arg->str);
        return 1;
    }

    return 0;
}

void ublox_sara_process_urc(ublox_sara_dev_t *dev, const char *str)
{
    ublox_sara_urc_handler_arg_t handler_arg = {
        .dev = dev,
        .str = str,
    };
    clist_foreach(&dev->urc_list, _check_urc, (void *)&handler_arg);
}

static void _handle_creg_urc(ublox_sara_dev_t *dev, const char *str)
{
    (void)dev;
    (void)str;
}

void ublox_sara_send_str(ublox_sara_dev_t *dev, const char *str)
{
    _debug_print_chars(dev, str, false);
    uart_write(dev->uart, (const uint8_t *)str, strlen(str));
}

void ublox_sara_send_char(ublox_sara_dev_t *dev, char c)
{
    _debug_print_char(dev, c, false);
    uart_write(dev->uart, (const uint8_t *)&c, 1);
}
