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
CFLAGS += -DAT45DB_PARAM_SPI_DEV='$(TEST_AT45DB_SPI_DEV)'
CFLAGS += -DAT45DB_PARAM_SPI_CS='$(TEST_AT45DB_SPI_CS)'
CFLAGS += -DAT45DB_PARAM_SPI_CLK='$(TEST_AT45DB_SPI_SPEED)'
CFLAGS += -DAT45DB_PARAM_VARIANT='$(TEST_AT45DB_VARIANT)'

include $(RIOTBASE)/Makefile.include
