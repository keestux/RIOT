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
#include <string.h>

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

static kernel_pid_t poller_pid;
static char poller_stack[THREAD_STACKSIZE_MAIN];

static void dump_ubx(eva8m_t* dev)
{
    uint8_t* ptr = dev->buffer;
    eva8m_class_id_t msg_class_id;
    msg_class_id = eva8m_received_class_id(dev);
    if (msg_class_id == UBX_ACK_ACK) {
        printf("ACK_ACK: class=%02X id=%02X\n", dev->buffer[2], dev->buffer[3]);
    }
    else if (msg_class_id == UBX_ACK_NAK) {
        printf("ACK_NAK: class=%02X id=%02X\n", dev->buffer[2], dev->buffer[3]);
    }
    else if (msg_class_id == UBX_NAV_PVT) {
        eva8m_nav_pvt_t pckt;
        memcpy(&pckt, &dev->buffer[6], sizeof(pckt));
        printf("NAV_PVT:\n");
        printf("    iTOW=%lu\n", (unsigned long)pckt.iTOW);
        printf("    date: %4u-%02u-%02u %02u:%02u:%02u\n",
               pckt.year, pckt.month, pckt.day, pckt.hour, pckt.minute, pckt.seconds);
        printf("    valid=%02X\n", pckt.valid);
        printf("    tAcc=%lu\n", (unsigned long)pckt.tAcc);
        printf("    nano=%ld\n", (long)pckt.nano);
        printf("    fixType=%02X\n", pckt.fixType);
        printf("    numSV=%u\n", (unsigned int)pckt.numSV);
    }
    else {
        printf("%02X", *ptr++);
        printf(" %02X", *ptr++);
        printf(" class=%02X", *ptr++);
        printf(" id=%02X", *ptr++);
        uint16_t length = *ptr++;
        length |= (*ptr++ >> 8);
        printf(" length=%u", length);
        for (uint16_t ix = 0; ix < length; ix++) {
            if (((ix % 8) == 0)) {
                printf("\n");
            }
            printf(" %02X", *ptr++);
        }
        printf("\n");
    }
}

static void *poller(void *arg)
{
    eva8m_t* dev = (eva8m_t*)arg;
    eva8m_portconfig_t portcfg;

    printf("poller\n");
    if (dev) {
        int result;
        /* Start by switching off outNmea */
        result = eva8m_get_port_config(dev, &portcfg);
        if (result == 0) {
            portcfg.outProtoMask = 1;
            //portcfg.outProtoMask &= ~(2);
        }
        result = eva8m_send_ubx_packet(dev, UBX_CFG_PRT, (uint8_t*)&portcfg, sizeof(portcfg));

        printf("UBX_NAV_PVT\n");
        result = eva8m_send_cfg_msg(dev, UBX_NAV_PVT, 1);
        result = eva8m_receive_ubx_packet(dev);
        dump_ubx(dev);

        printf("UBX_NAV_SAT\n");
        result = eva8m_send_cfg_msg(dev, UBX_NAV_SAT, 10);
        result = eva8m_receive_ubx_packet(dev);
        dump_ubx(dev);

        while (1) {
            result = eva8m_receive_ubx_packet(dev);
            if (result == 0) {
                if (dev->prot == EVA8M_PROT_NMEA) {
                    printf("%s\n", dev->buffer);
                }
                else if (dev->prot == EVA8M_PROT_UBX) {
                    dump_ubx(dev);
                }
                else {
                    printf("-- unknown packet --\n");
                }
            }
        }
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
    poller_pid = thread_create(poller_stack, sizeof(poller_stack),
                               POLLER_PRIO, 0, poller, &dev, "poller");

    while (1) {
        xtimer_usleep(2 * 1000 * 1000u);
    }
    return 0;
}
