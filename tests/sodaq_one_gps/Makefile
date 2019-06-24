# name of your application
APPLICATION = sodaq_one_gps

# If no BOARD is found in the environment, use this default:
BOARD ?= sodaq_one

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

FEATURES_REQUIRED += periph_gpio_irq

include $(RIOTBASE)/Makefile.include
