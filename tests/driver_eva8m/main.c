/*
 * Copyright (C) 2018 Kees Bakker, SODAQ
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
 * @brief       Test application for the u-blox EVA 8/8M.
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <stdio.h>
#include <inttypes.h>

#include "eva8m_params.h"
#include "eva8m.h"
#include "msg.h"
#include "thread.h"
#include "ringbuffer.h"
#include "xtimer.h"

#define MAINLOOP_DELAY  (2 * 1000 * 1000u)      /* 2 seconds delay between printf's */

#define BUFSIZE         (128U)
static char rx_buf[BUFSIZE];
static ringbuffer_t rx_ringbuf;

#define POLLER_PRIO     (THREAD_PRIORITY_MAIN - 1)
#define POLLER2_PRIO    (THREAD_PRIORITY_MAIN - 2)

static kernel_pid_t poller_pid;
static char poller_stack[THREAD_STACKSIZE_MAIN];

static kernel_pid_t poller2_pid;
static char poller2_stack[THREAD_STACKSIZE_MAIN];

static void *poller(void *arg)
{
    eva8m_t* dev = (eva8m_t*)arg;
    int counter = 0;

    printf("poller\n");
    if (dev) {
        while (1) {
            int result;
            uint16_t nr_avail;
            result = eva8m_available(dev, &nr_avail);
            if (result == 0) {
                while (nr_avail > 0 && !ringbuffer_full(&rx_ringbuf)) {
                    uint8_t b;
                    eva8m_read_byte(dev, &b);
                    nr_avail--;
                    if (b != 0xFF) {
                        char c = (char)b;
                        (void)ringbuffer_add_one(&rx_ringbuf, c);
                        if (c == '\n') {
                            msg_t msg;
                            msg_send(&msg, poller2_pid);
                        }
                    }
                }
            }
            if (++counter > 100) {
                eva8m_portconfig_t portcfg;
                counter = 0;
                result = eva8m_get_port_config(dev, &portcfg);
            }
            // Wait a bit to avoid wasting cpu cycles
            // TODO. How long should we wait?
            xtimer_usleep(10 * 1000u);
        }
    }

    /* this should never be reached */
    return NULL;
}

static void *poller2(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, sizeof(msg_queue)/sizeof(msg_queue[0]));

    printf("poller2\n");
    while (1) {
        msg_receive(&msg);
        int c;
        do {
            c = ringbuffer_get_one(&rx_ringbuf);
            if (c < 0) {
                /* buffer empty */
                printf("\n<empty>");
                break;
            }
            else if (c == '\n'
                     || c == '\r'
                     || (c >= ' ' && c <= '~')) {
                putchar(c);
            }
            else {
                printf("\\x%02x", (unsigned char)c);
            }
        } while (c != '\n');
    }

    /* this should never be reached */
    return NULL;
}

int main(void)
{
    eva8m_t dev;
    int result;

    puts("EVA8M test application\n");

    GPS_ENABLE_ON;
    //xtimer_usleep(MAINLOOP_DELAY);

    result = eva8m_init(&dev, &eva8m_params[0]);
    if (result < 0) {
        puts("[Error] Did not detect an EVA 8/8M");
        return 1;
    }

    /* initialize ringbuffer(s) */
    ringbuffer_init(&rx_ringbuf, rx_buf, BUFSIZE);

    /* start the poller thread */
    poller2_pid = thread_create(poller2_stack, sizeof(poller2_stack),
                               POLLER2_PRIO, 0, poller2, NULL, "poller2");
    poller_pid = thread_create(poller_stack, sizeof(poller_stack),
                               POLLER_PRIO, 0, poller, &dev, "poller");

    while (1) {
        xtimer_usleep(2 * 1000 * 1000u);
    }
    return 0;
}
