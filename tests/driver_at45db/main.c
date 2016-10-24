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

static int cmd_read_page(int argc, char **argv);
static const shell_command_t shell_commands[] = {
    { "rp", "Read a page", cmd_read_page },
    { NULL, NULL, NULL }
};

static void dump_buffer(const char *txt, uint8_t *buffer, size_t size);

int main(void)
{
    puts("AT45DB test application starting...");

    puts("Initializing AT45DB device descriptor... ");
    if (at45db_init(&dev, TEST_AT45DB_SPI_DEV, TEST_AT45DB_SPI_CS, &TEST_AT45DB_DETAILS) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]\n");
        return 1;
    }

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

    size_t bufno = 1;
    uint8_t *buffer;
    /* TODO size of buffer depends on selected DataFlash chip */
    const size_t buffer_size = 526;
    int page_nr = atoi(argv[1]);
    buffer = malloc(buffer_size);

    if (at45db_page2buf(&dev, page_nr, bufno) < 0) {
        printf("ERROR: cannot read page #%d to buf#%d\n", page_nr, bufno);
        return 1;
    }

    if (at45db_read_buf(&dev, bufno, buffer, buffer_size) < 0) {
        printf("ERROR: cannot read buf#%d\n", bufno);
        return 1;
    }

    dump_buffer("page", buffer, buffer_size);

    free(buffer);

    return 0;
}

static void dump_buffer(const char *txt, uint8_t *buffer, size_t size)
{
    size_t ix;
    for (ix = 0; ix < size; ix++) {
        if (ix != 0 && (ix % 16) == 0) {
            printf("\n");
        }
        printf("%02X", buffer[ix]);
    }
    if ((ix % 16) != 0) {
        printf("\n");
    }
}
