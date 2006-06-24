##########################################################################
#
#    common make rules and variables
#
#    (c) 2001-2002 Gilles Boccon-Gibod
#    Author: Gilles Boccon-Gibod (bok@bok.net)
#
##########################################################################

##########################################################################
# build configurations
##########################################################################
VPATH += $(AP4_BUILD_CONFIG)

COMPILE_CPP_OPTIONS = $(WARNINGS_CPP) 

ifeq ($(AP4_BUILD_CONFIG),Profile)
COMPILE_CPP_OPTIONS += $(PROFILE_CPP)
endif
ifeq ($(AP4_BUILD_CONFIG),Debug)
COMPILE_CPP_OPTIONS += $(DEBUG_CPP)
else
COMPILE_CPP_OPTIONS += $(OPTIMIZE_CPP)
endif

##########################################################################
# default rules
##########################################################################
%.d: %.cpp
	$(AUTODEP_CPP) $(DEFINES_CPP) $(INCLUDES_CPP) $< -o $@

ifneq ($(AUTODEP_STDOUT),)
%.d: %.c
	$(AUTODEP_C) $(DEFINES_C) $(INCLUDES_C) $< > $@
else
%.d: %.c
	$(AUTODEP_CPP) $(DEFINES_CPP) $(INCLUDES_CPP) $< -o $@
endif

%.o: %.cpp
	$(COMPILE_CPP) $(COMPILE_CPP_OPTIONS) $($@_LOCAL_DEFINES_CPP) $(DEFINES_CPP) $(INCLUDES_CPP) -c $< -o $@

%.a:
	$(ARCHIVE) -o $@ $^

.PHONY: clean
clean:
	@rm -rf $(TO_CLEAN)

TITLE = @echo ============ making $@ =============
INVOKE_SUBMAKE = $(MAKE) --no-print-directory

##########################################################################
# variables
##########################################################################
LINK                 = $(LINK_CPP) $(LINK_CPP_OPTIONS) 
LINK_CPP_OPTIONS    += $(foreach lib,$(TARGET_LIBRARIES),-l$(lib))
TARGET_LIBRARY_FILES = $(foreach lib,$(TARGET_LIBRARIES),lib$(lib).a)
TARGET_OBJECTS       = $(TARGET_SOURCES:.cpp=.o)

##########################################################################
# auto dependencies
##########################################################################
TARGET_DEPENDENCIES := $(patsubst %.c,%.d,$(TARGET_SOURCES))
TARGET_DEPENDENCIES := $(patsubst %.cpp,%.d,$(TARGET_DEPENDENCIES))

ifneq ($(TARGET_DEPENDENCIES),)
include $(TARGET_DEPENDENCIES)
endif

##########################################################################
# includes
##########################################################################
ifneq ($(LOCAL_RULES),)
include $(LOCAL_RULES)
endif
