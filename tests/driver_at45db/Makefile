APPLICATION = driver_at45db
include ../Makefile.tests_common

USEMODULE += at45db
USEMODULE += xtimer

# set default device parameters in case they are undefined
TEST_AT45DB_SPI_DEV           ?= SPI_0
TEST_AT45DB_SPI_CS            ?= GPIO_PIN\(0,23\)
TEST_AT45DB_SPI_SPEED  	      ?= SPI_SPEED_10MHZ

# export parameters
CFLAGS += -DTEST_AT45DB_SPI_DEV=$(TEST_AT45DB_SPI_DEV)
CFLAGS += -DTEST_AT45DB_SPI_CS=$(TEST_AT45DB_SPI_CS)
CFLAGS += -DTEST_AT45DB_SPI_SPEED=$(TEST_AT45DB_SPI_SPEED)

include $(RIOTBASE)/Makefile.include
