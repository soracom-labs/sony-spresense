include $(APPDIR)/Make.defs
include $(SDKDIR)/Make.defs

# lte_at_cmd built-in application info

PROGNAME = $(CONFIG_EXAMPLES_LTE_AT_CMD_PROGNAME)
PRIORITY = $(CONFIG_EXAMPLES_LTE_AT_CMD_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_LTE_AT_CMD_STACKSIZE)
MODULE   = $(CONFIG_EXAMPLES_LTE_AT_CMD_SAVE)

# Command name (Public function 'int <APPNAME>_main(void)' required)
APPNAME = lte_at_cmd

# Main source code
MAINSRC = lte_at_cmd_main.c

include $(APPDIR)/Application.mk
