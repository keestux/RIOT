/*
 * Copyright (C) 2024 Kees Bakker <kees@ijzerbout.nl>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief    Ublox SARA module test application
 *
 * @author   Kees Bakker <kees@ijzerbout.nl>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ublox_sara.h"

#include "board.h"
#include "periph/gpio.h"
#include "shell.h"
#include "timex.h"
#include "ztimer.h"

static ublox_sara_dev_t ublox_sara_dev;

/**
 * @brief Buffer for RX interrupt handling
 */
static char rx_isr_buf[256];

static char resp_buffer[1024];

static int init(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: %s <uart> <baudrate>\n", argv[0]);
        return 1;
    }

    uint8_t uart = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);

    if (uart >= UART_NUMOF) {
        printf("Wrong UART device number - should be in range 0-%d.\n", UART_NUMOF - 1);
        return 1;
    }

    int res = ublox_sara_dev_init(&ublox_sara_dev, UART_DEV(uart), baudrate, rx_isr_buf, sizeof(rx_isr_buf));

    /* check the UART initialization return value and respond as needed */
    if (res == UART_NODEV) {
        puts("Invalid UART device given!");
        return 1;
    }
    else if (res == UART_NOBAUD) {
        puts("Baudrate is not applicable!");
        return 1;
    }

    ublox_sara_power_off(&ublox_sara_dev);
    ztimer_sleep(ZTIMER_MSEC, 1000);
    ublox_sara_power_on(&ublox_sara_dev);
    if (ublox_sara_is_alive(&ublox_sara_dev, 6)) {

        char *commands[] = {
            "AT",
            "ati",
            "at+ipr=?",
            "AT+CCID",
            "AT+CIMI",
            "AT+CGMI",
            "AT+CGMM",
            "AT+CGMR",
            "ATI9",
            "AT+CSQ",
        };
        for (size_t ix = 0; ix < ARRAY_SIZE(commands); ix++) {
            ublox_sara_send_cmd_wait_ok(&ublox_sara_dev, commands[ix], resp_buffer, sizeof(resp_buffer), 10 * US_PER_SEC);
        }
    }
    else {
        uint32_t current_baudrate = ublox_sara_determine_baudrate(&ublox_sara_dev, 0, ublox_sara_r4x_baudrates);
        printf("Determined baudrate: %" PRIu32 "\n", current_baudrate);

        printf("Now changing to baudrate: %" PRIu32 "\n", baudrate);
        res = ublox_sara_change_baudrate(&ublox_sara_dev, baudrate);
    }

    return res;
}

static int send(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        return 1;
    }

    ublox_sara_send_cmd(&ublox_sara_dev, argv[1]);

    return 0;
}

static int send_ok(int argc, char **argv)
{
    int res;
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        return 1;
    }

    res = ublox_sara_send_cmd_wait_ok(&ublox_sara_dev, argv[1], resp_buffer, sizeof(resp_buffer), 10 * US_PER_SEC);
    if (res == UBLOX_SARA_OK) {
        printf("%s\n", resp_buffer);
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "init", "Initialize Ublox SARA device", init },
    { "send", "Send a command", send },
    { "send_ok", "Send a command and wait for OK", send_ok },
    { NULL, NULL, NULL },
};

int main(void)
{
    puts("Ublox SARA test app");

    // /* Some initial commands for Kees, with his SODAQ SARA SFF with Ublox-N310/R410 */
    /* R4X default 115200 */
    /* N3X default 38400 */
    char *init_argv[4] = {"init", "1", "115200", NULL};
    init(3, init_argv);

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
