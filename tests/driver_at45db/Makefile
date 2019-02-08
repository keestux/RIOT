APPLICATION = driver_at45db
include ../Makefile.tests_common

USEMODULE += at45db
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += xtimer

# set default device parameters in case they are undefined
TEST_AT45DB_SPI_DEV           ?= SPI_DEV(0)
TEST_AT45DB_SPI_CS            ?= GPIO_PIN(0,23)
TEST_AT45DB_SPI_SPEED         ?= SPI_CLK_10MHZ
TEST_AT45DB_VARIANT           ?= AT45DB161E

# export parameters
CFLAGS += -DTEST_AT45DB_SPI_DEV='$(TEST_AT45DB_SPI_DEV)'
CFLAGS += -DTEST_AT45DB_SPI_CS='$(TEST_AT45DB_SPI_CS)'
CFLAGS += -DTEST_AT45DB_SPI_SPEED='$(TEST_AT45DB_SPI_SPEED)'
CFLAGS += -DTEST_AT45DB_VARIANT='$(TEST_AT45DB_VARIANT)'

include $(RIOTBASE)/Makefile.include
