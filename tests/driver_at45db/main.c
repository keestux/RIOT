/*
 * Copyright (C) 2019 Kees Bakker, SODAQ
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
#include "at45db_params.h"
#include "mtd_at45db.h"
#include "mtd_sdcard.h"
#include "board.h"
#include "periph/spi.h"
#include "shell.h"
#include "xtimer.h"

static at45db_t dev;

#if defined(MODULE_MTD_AT45DB) || defined(DOXYGEN)
 /* this is provided by the at45db driver
 * see sys/auto_init/storage/auto_init_at45db.c
 */
extern at45db_t at45db_devs[sizeof(at45db_params) /
                            sizeof(at45db_params[0])];
mtd_at45db_t my_mtd_dev = {
    .base = {
        .driver = &mtd_at45db_driver,
        .page_size = 256,       /* TODO. This depends on AT45DB variant */
        .pages_per_sector = 1,  /* TODO What is this? */
        .sector_count = 4096,   /* TODO Not sure. Nr pages of AT45DB161E: 4096 */
    },
    .at45db_dev = &at45db_devs[0],
    .params = &at45db_params[0]
};

mtd_dev_t *mtd0 = (mtd_dev_t *)&my_mtd_dev;
#endif /* MODULE_MTD_AT45DB || DOXYGEN */

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
    if (at45db_init(&dev, &at45db_params[0]) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]\n");
        return 1;
    }
    printf("SPI clock = %ld\n", (long)dev.params.clk);

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

    return 0;
}

static int cmd_read_all_pages(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return 0;
}

static int cmd_erase_page(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <page no>\n", argv[0]);
        return 1;
    }

    return 0;
}

static int cmd_security_register(int argc, char **argv)
{
    (void)argc;
    (void)argv;

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
    (void)argc;
    (void)argv;

    enable_dump_buffer = false;
    return 0;
}

static int cmd_enable_dump(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    enable_dump_buffer = true;
    return 0;
}

static void dump_buffer(const char *txt, uint8_t *buffer, size_t size)
{
    printf("%s:\n", txt);
    size_t ix;
    for (ix = 0; ix < size; ix++) {
        printf("%02X", buffer[ix]);
        if ((ix + 1) == size || (((ix + 1) % 16) == 0)) {
            printf("\n");
        }
    }
}
