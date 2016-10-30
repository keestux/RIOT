/*
 * Copyright (C) 2016 Kees Bakker, SODAQ
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
 * @brief       Test application for the AT45DB driver
 *
 * @author      Kees Bakker <kees@sodaq.com>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "at45db.h"
#include "board.h"
#include "periph/spi.h"
#include "shell.h"
#include "xtimer.h"

#ifdef TEST_AT45DB_SPI_CONF
#define SPI_CONF    (TEST_AT45DB_SPI_CONF)
#else
#define SPI_CONF    (SPI_CONF_FIRST_RISING)
#endif

#ifdef TEST_AT45DB_SPI_SPEED
#define SPI_SPEED   (TEST_AT45DB_SPI_SPEED)
#else
#define SPI_SPEED   (SPI_SPEED_10MHZ)
#endif

static at45db_t dev;

static int cmd_read_all_pages(int argc, char **argv);
static int cmd_read_page(int argc, char **argv);
static int cmd_erase_page(int argc, char **argv);
static int cmd_security_register(int argc, char **argv);
static int cmd_disable_dump(int argc, char **argv);
static int cmd_enable_dump(int argc, char **argv);
static const shell_command_t shell_commands[] = {
    { "rall", "Read all pages", cmd_read_all_pages },
    { "rp", "Read a page", cmd_read_page },
    { "ep", "Erase a page", cmd_erase_page },
    { "sr", "Read Security Register", cmd_security_register },
    { "dis", "Disable dump", cmd_disable_dump },
    { "ena", "Enable dump", cmd_enable_dump },
    { NULL, NULL, NULL }
};

static bool enable_dump_buffer;
static void dump_buffer(const char *txt, uint8_t *buffer, size_t size);

int main(void)
{
    puts("AT45DB test application starting...");

    puts("Initializing AT45DB device descriptor... ");
    if (at45db_init(&dev, TEST_AT45DB_SPI_DEV, TEST_AT45DB_SPI_CS, TEST_AT45DB_VARIANT) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]\n");
        return 1;
    }
    dev.clk = TEST_AT45DB_SPI_SPEED;
    printf("SPI clock = %ld\n", (long)dev.clk);

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}

static int cmd_read_page(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <page no>\n", argv[0]);
        return 1;
    }

    uint32_t start;
    size_t bufno = 1;
    uint8_t *buffer;
    const size_t buffer_size = at45db_get_page_size(&dev);
    int page_nr = atoi(argv[1]);
    buffer = malloc(buffer_size);

    start = xtimer_now();
    if (at45db_page2buf(&dev, page_nr, bufno) < 0) {
        printf("ERROR: cannot read page #%d to buf#%d\n", page_nr, bufno);
        free(buffer);
        return 1;
    }
    printf("at45db_page2buf time = %lu\n", (long)(xtimer_now() - start));

    start = xtimer_now();
    if (at45db_read_buf(&dev, bufno, 0, buffer, buffer_size) < 0) {
        printf("ERROR: cannot read buf#%d\n", bufno);
        free(buffer);
        return 1;
    }
    printf("at45db_read_buf time = %lu\n", (long)(xtimer_now() - start));

    if (enable_dump_buffer) {
        dump_buffer("page", buffer, buffer_size);
    }

    free(buffer);

    return 0;
}

static int cmd_read_all_pages(int argc, char **argv)
{
    uint32_t start;
    size_t bufno = 1;
    uint8_t *buffer;
    const size_t buffer_size = at45db_get_page_size(&dev);
    const size_t nr_pages = at45db_get_nr_pages(&dev);
    buffer = malloc(buffer_size);

    start = xtimer_now();
    for (size_t page = 0; page < nr_pages; page++) {
        if (!enable_dump_buffer) {
            if ((page % 16) == 0) {
                putchar('.');
                fflush(stdout);
            }
        }

        if (at45db_page2buf(&dev, page, bufno) < 0) {
            printf("ERROR: cannot read page #%d to buf#%d\n", page, bufno);
            free(buffer);
            return 1;
        }

        if (at45db_read_buf(&dev, bufno, 0, buffer, buffer_size) < 0) {
            printf("ERROR: cannot read buf#%d\n", bufno);
            free(buffer);
            return 1;
        }

        if (enable_dump_buffer) {
            /* Only 16 bytes */
            dump_buffer("page", buffer, 16);
        }
    }
    printf("\n");
    printf("reading all pages, time = %lu\n", (long)(xtimer_now() - start));

    free(buffer);

    return 0;
}

static int cmd_erase_page(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <page no>\n", argv[0]);
        return 1;
    }

    int page_nr = atoi(argv[1]);

    if (at45db_erase_page(&dev, page_nr) < 0) {
        printf("ERROR: cannot erase page #%d\n", page_nr);
        return 1;
    }

    return 0;
}

static int cmd_security_register(int argc, char **argv)
{
    uint8_t *buffer;
    const size_t buffer_size = 128;
    buffer = malloc(buffer_size);

    if (at45db_security_register(&dev, buffer, buffer_size) < 0) {
        printf("ERROR: cannot read security register\n");
        free(buffer);
        return 1;
    }

    dump_buffer("security register", buffer, buffer_size);

    free(buffer);

    return 0;
}

static int cmd_disable_dump(int argc, char **argv)
{
    enable_dump_buffer = false;
    return 0;
}

static int cmd_enable_dump(int argc, char **argv)
{
    enable_dump_buffer = true;
    return 0;
}

static void dump_buffer(const char *txt, uint8_t *buffer, size_t size)
{
    size_t ix;
    for (ix = 0; ix < size; ix++) {
        printf("%02X", buffer[ix]);
        if ((ix + 1) == size || (((ix + 1) % 16) == 0)) {
            printf("\n");
        }
    }
}
