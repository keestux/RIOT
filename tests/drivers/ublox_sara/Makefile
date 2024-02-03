include ../Makefile.drivers_common

USEMODULE += shell
USEMODULE += ublox_sara
USEMODULE += ztimer
USEMODULE += ztimer_msec
USEMODULE += stdio_uart

# we are printing from the event thread, we need more stack
CFLAGS += -DEVENT_THREAD_MEDIUM_STACKSIZE=1024
CFLAGS += -DUBLOX_SARA_DEBUG=1
CFLAGS += -DSTDIO_UART_BAUDRATE=460800
CFLAGS += -DSTDIO_UART_DEV='UART_DEV(0)'

include $(RIOTBASE)/Makefile.include
