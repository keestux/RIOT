include ../Makefile.drivers_common

USEMODULE += shell
USEMODULE += at
USEMODULE += at_urc_isr_medium
USEMODULE += ztimer
USEMODULE += ztimer_msec
USEMODULE += stdio_uart

# we are printing from the event thread, we need more stack
CFLAGS += -DEVENT_THREAD_MEDIUM_STACKSIZE=1024
CFLAGS += -DAT_PRINT_INCOMING=1
CFLAGS += -DAT_PRINT_OUTGOING=1
CFLAGS += -DSTDIO_UART_BAUDRATE=115200
CFLAGS += -DSTDIO_UART_DEV='UART_DEV(0)'

include $(RIOTBASE)/Makefile.include
