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
#include <time.h>

#include "board.h"
#include "eva8m_params.h"
#include "eva8m.h"
#include "periph/gpio.h"
#include "thread.h"
#include "ringbuffer.h"
#include "ztimer.h"
#include "rtc_utils.h"

#define MAINLOOP_DELAY  (2 * 1000)      /**< 2 seconds delay between printf's */

#define MAIN_MSG_QUEUE_SIZE (4)
static msg_t main_msg_queue[MAIN_MSG_QUEUE_SIZE];
#define MSG_TYPE_PPS_INTERRUPT   1      /**< msg.type for PPS interrupt */
#define MSG_TYPE_BTN0_INTERRUPT  2      /**< msg.type for BTN0 interrupt */
#define MSG_TYPE_LED0_OFF        3      /**< msg.type for LED0 off */
#define MSG_TYPE_LED1_OFF        4      /**< msg.type for LED1 off */
#define MSG_TYPE_LED2_OFF        5      /**< msg.type for LED2 off */

#define BUFSIZE         (128U)
static char rx_buf[BUFSIZE];
static ringbuffer_t rx_ringbuf;

#define POLLER_PRIO     (THREAD_PRIORITY_MAIN + 1)

static kernel_pid_t poller_pid;
static char poller_stack[THREAD_STACKSIZE_MAIN];

static void dump_ubx(eva8m_t* dev)
{
    uint8_t* ptr = dev->buffer;
    eva8m_class_id_t msg_class_id = eva8m_received_class_id(dev);
    static uint32_t prev_ts;

    if (msg_class_id == UBX_ACK_ACK) {
        printf("ACK_ACK: class=%02X id=%02X\n", dev->buffer[2], dev->buffer[3]);
    }
    else if (msg_class_id == UBX_ACK_NAK) {
        printf("ACK_NAK: class=%02X id=%02X\n", dev->buffer[2], dev->buffer[3]);
    }
    else if (msg_class_id == UBX_NAV_PVT) {
        static uint32_t prev_pps_counter;
        uint32_t elapsed_pps = dev->pps_counter - prev_pps_counter;
        prev_pps_counter = dev->pps_counter;

        eva8m_nav_pvt_t pckt;
        memcpy(&pckt, &dev->buffer[6], sizeof(pckt));

        struct tm datetime;
        memset(&datetime, 0, sizeof(datetime));
        datetime.tm_year = pckt.year - 1900;
        datetime.tm_mon = pckt.month - 1;
        datetime.tm_mday = pckt.day;
        datetime.tm_hour = pckt.hour;
        datetime.tm_min = pckt.min;
        datetime.tm_sec = pckt.sec;
        // rtc_tm_normalize(&datetime);

        printf("NAV_PVT:");
        printf(" iTOW=%lu", (unsigned long)pckt.iTOW);
        printf(" date: %4u-%02u-%02u %02u:%02u:%02u",
               pckt.year, pckt.month, pckt.day, pckt.hour, pckt.min, pckt.sec);
        printf(" valid=%u,%u,%u,%u", pckt.validDate, pckt.validTime, pckt.fullyResolved, pckt.validMag);
        printf(" tAcc=%lu", (unsigned long)pckt.tAcc);
        printf(" nano=%ld", (long)pckt.nano);
        printf(" fixType=%02X", pckt.fixType);
        printf(" flags=%02X", pckt.flags);
        printf(" flags2=%02X", pckt.flags2);
        printf(" numSV=%u\n", (unsigned int)pckt.numSV);

        uint32_t ts = rtc_mktime(&datetime);
        uint32_t elapsed_seconds = ts - prev_ts;
        if (elapsed_seconds > elapsed_pps) {
            printf("Missed %lu PPS pulses\n", elapsed_seconds - elapsed_pps);
        }
        prev_ts = ts;
    }
    else if (msg_class_id == UBX_MON_VER) {
        int offset = 6;
        printf("MON_VER:\n");
        printf("    swVersion: '%s'\n", (char *)&dev->buffer[offset + 0]);
        printf("    hwVersion: '%s'\n", (char *)&dev->buffer[offset + 30]);
        printf("    extension0: '%s'\n", (char *)&dev->buffer[offset + 40]);
        printf("    extension1: '%s'\n", (char *)&dev->buffer[offset + 70]);
        printf("    extension2: '%s'\n", (char *)&dev->buffer[offset + 100]);
        printf("    extension3: '%s'\n", (char *)&dev->buffer[offset + 130]);
    }
    else if (msg_class_id == UBX_NAV_SAT) {
        printf("NAV_SAT:\n");
        eva8m_nav_sat_head_t pckt;
        // Length: 8 + 12 * numSvs
        memcpy(&pckt, &dev->buffer[6], sizeof(pckt));
        printf("    iTOW=%lu\n", (unsigned long)pckt.iTOW);
        printf("    version=%02X\n", (unsigned int)pckt.version);
        printf("    numSvs=%u\n", (unsigned int)pckt.numSvs);
        for (uint8_t ix = 0; ix < pckt.numSvs; ix++) {
            eva8m_nav_sat_sv_t pckt_sv;
            memcpy(&pckt_sv, &dev->buffer[6 + 8 + (12 * ix)], sizeof(pckt_sv));
            printf("     sv %u:", ix);
            printf(" gnssId=%02X", (unsigned int)pckt_sv.gnssId);
            printf(" svId=%02X", (unsigned int)pckt_sv.svId);
            printf(" cno=%02X", (unsigned int)pckt_sv.cno);
            printf(" elev=%d", (int)pckt_sv.elev);
            printf(" azim=%d", (int)pckt_sv.azim);
            printf(" psRes=%d\n", (int)pckt_sv.prRes);
        }
    }
    else {
        printf("other:\n");
        printf("    %02X", *ptr++);
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

static void dump_timepulse_parm(eva8m_timepulseparm_t *parm)
{
    printf("CFG_TP5:\n");
    printf("    tpIdx=%u\n", (unsigned)parm->tpIdx);
    printf("    version=%u\n", (unsigned)parm->version);
    printf("    antCableDelay=%u\n", (unsigned)parm->antCableDelay);
    printf("    rfGroupDelay=%u\n", (unsigned)parm->rfGroupDelay);
    printf("    freqPeriod=%lu\n", (unsigned long)parm->freqPeriod);
    printf("    freqPeriodLock=%lu\n", (unsigned long)parm->freqPeriodLock);
    printf("    flags=0x%08lx\n", (unsigned long)parm->flags);
}

static void *poller(void *arg)
{
    eva8m_t* dev = (eva8m_t*)arg;
    eva8m_portconfig_t portcfg;
    eva8m_timepulseparm_t timepulse_parm;

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
        /* ACK / NACK */
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        if (result == 0 && eva8m_received_class_id(dev) == UBX_ACK_ACK) {
            // printf("[EVA8M] received ACK\n");
        }
        else if (eva8m_received_class_id(dev) == UBX_ACK_NAK) {
            printf("[EVA8M] received NACK\n");
        }

        result = eva8m_get_timepulse_parm(dev, &timepulse_parm);
        if (result == 0) {
            dump_timepulse_parm(&timepulse_parm);
        }
#if 1
        /* NAV-PVT every 10 seconds */
        printf("UBX_NAV_PVT\n");
        result = eva8m_send_cfg_msg(dev, UBX_NAV_PVT, 10);
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        dump_ubx(dev);

        /* NAV-SAT Every 100 seconds */
        printf("UBX_NAV_SAT\n");
        result = eva8m_send_cfg_msg(dev, UBX_NAV_SAT, 100);
        result = eva8m_receive_ubx_packet(dev, EVA8M_DEFAULT_TIMEOUT);
        dump_ubx(dev);
#endif

        (void)eva8m_send_ubx_packet(dev, UBX_MON_VER, NULL, 0);

        /* run forever */
        while (1) {
            result = eva8m_receive_ubx_packet(dev, 1100);
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
            thread_yield_higher();
        }
    }

    /* this should never be reached */
    return NULL;
}

/**
 * @brief   Timepulse (PPS) call back
 *
 * @param[in] arg           Call back argument, pointer to a eva8m dev.
 *
 * Careful. This is called in interrupt context.
 */
static void timepulse_cb(void *arg)
{
    eva8m_t * dev = arg;
    // printf(">> GPS Timepulse (pid=%d)\n", dev->pps_thread_pid);
    dev->pps_counter++;

    msg_t msg;
    msg.type = MSG_TYPE_PPS_INTERRUPT;
    msg.content.ptr = dev;
    msg_send_int(&msg, dev->pps_thread_pid);
}

/**
 * @brief   Button press (BTN0) call back
 *
 * @param[in] arg           Call back argument, pointer to a counter.
 *
 * Careful. This is called in interrupt context.
 */
static void button0_cb(void *arg)
{
    eva8m_t * dev = arg;
    // printf(">> BTN0 (pid=%d)\n", dev->pps_thread_pid);
    dev->btn0_counter++;

    msg_t msg;
    msg.type = MSG_TYPE_BTN0_INTERRUPT;
    msg.content.ptr = dev;
    msg_send_int(&msg, dev->pps_thread_pid);
    // msg_send_int(&msg, KERNEL_PID_FIRST);
}

int main(void)
{
    eva8m_t dev;
    int result;
    ztimer_t timer;

    msg_init_queue(main_msg_queue, MAIN_MSG_QUEUE_SIZE);

#ifdef GPS_ENABLE_ON
    GPS_ENABLE_ON;
#endif
    ztimer_sleep(ZTIMER_MSEC, MAINLOOP_DELAY);

    puts("EVA8M test application\n");
    // printf("main pid=%d\n", thread_getpid());

    result = eva8m_init(&dev, &eva8m_params[0]);
    if (result < 0) {
        puts("[Error] Did not detect an EVA 8/8M");
        return 1;
    }

    if (gpio_init_int(GPS_TIMEPULSE_PIN, GPS_TIMEPULSE_MODE, GPIO_RISING, timepulse_cb, (void *)&dev) < 0) {
        puts("[FAILED] init GPS_TIMEPULSE!");
        return 1;
    }

    if (gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_FALLING, button0_cb, (void *)&dev) < 0) {
        puts("[FAILED] init BTN0!");
        return 1;
    }

    /* initialize ringbuffer(s) */
    ringbuffer_init(&rx_ringbuf, rx_buf, BUFSIZE);

    /* start the poller thread */
    poller_pid = thread_create(poller_stack, sizeof(poller_stack),
                               POLLER_PRIO, 0, poller, &dev, "poller");

    /* run forever */
    while (1) {
        // printf("main loop ... (pid=%d)\n", thread_getpid());
        msg_t msg;
        msg_receive(&msg);
        switch (msg.type) {
        case MSG_TYPE_BTN0_INTERRUPT:
            printf("BTN0 pressed, counter=%lu\n", ((eva8m_t*)msg.content.ptr)->btn0_counter);
#ifdef LED0_ON
            LED0_ON;
            /* Set a timer to switch it off. */
            msg.type = MSG_TYPE_LED0_OFF;
            ztimer_set_msg(ZTIMER_MSEC, &timer, 10, &msg, thread_getpid());
#endif
            break;
        case MSG_TYPE_PPS_INTERRUPT:
            printf("PPS, counter=%lu\n", ((eva8m_t*)msg.content.ptr)->pps_counter);
#ifdef LED2_ON
            LED2_ON;
            /* Set a timer to switch it off. */
            msg.type = MSG_TYPE_LED2_OFF;
            ztimer_set_msg(ZTIMER_MSEC, &timer, 10, &msg, thread_getpid());
#endif
            break;
        case MSG_TYPE_LED0_OFF:
#ifdef LED0_OFF
            LED0_OFF;
#endif
            break;
        case MSG_TYPE_LED1_OFF:
#ifdef LED1_OFF
            LED1_OFF;
#endif
            break;
        case MSG_TYPE_LED2_OFF:
#ifdef LED2_OFF
            LED2_OFF;
#endif
            break;
        }
        // ztimer_sleep(ZTIMER_MSEC, 100);
    }
    return 0;
}
