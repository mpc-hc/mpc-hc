#######################################################################
#
#   bootstrap makefile
#
#######################################################################
AP4_BUILD_CONFIG = Debug

ifeq ($(MAKECMDGOALS),)
    MAKECMDGOALS = default
else
    TARGETS = $(MAKECMDGOALS)
endif

$(MAKECMDGOALS):
	@[ -d $(AP4_BUILD_CONFIG) ] || mkdir $(AP4_BUILD_CONFIG)
	@$(MAKE) -C $(AP4_BUILD_CONFIG) AP4_BUILD_CONFIG=$(AP4_BUILD_CONFIG) -f ../Local.mak $(TARGETS)