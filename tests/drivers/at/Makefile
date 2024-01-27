include ../Makefile.drivers_common

USEMODULE += shell
USEMODULE += at
USEMODULE += at_urc_isr_medium
USEMODULE += ztimer
USEMODULE += ztimer_msec

# we are printing from the event thread, we need more stack
CFLAGS += -DEVENT_THREAD_MEDIUM_STACKSIZE=1024
CFLAGS += -DAT_PRINT_INCOMING=1
CFLAGS += -DAT_PRINT_OUTGOING=1

include $(RIOTBASE)/Makefile.include
