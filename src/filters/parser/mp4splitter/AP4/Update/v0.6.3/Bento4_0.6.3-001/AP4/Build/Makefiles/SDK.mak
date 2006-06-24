##########################################################################
#
#    SDK Makefile
#
#    (c) 2001-2005 Gilles Boccon-Gibod
#
##########################################################################
all: sdk

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Crypto.exp
include $(BUILD_ROOT)/Makefiles/Core.exp
include $(BUILD_ROOT)/Makefiles/Codecs.exp
include $(BUILD_ROOT)/Makefiles/System.exp

##########################################################################
# variables
##########################################################################
SDK_LIBRARY  = SDK/lib/libAP4.a
SDK_HEADERS := $(SOURCE_ROOT)/Core/*.h $(SOURCE_ROOT)/Config/*.h $(SOURCE_ROOT)/Crypto/*.h $(SOURCE_ROOT)/Codecs/*.h
SDK_BINARIES = $(ALL_APPS)

##########################################################################
# rules
##########################################################################
.PHONY: sdk-dirs
sdk-dirs:
	@rm -rf SDK
	@mkdir SDK
	@mkdir SDK/lib
	@mkdir SDK/include
	@mkdir SDK/bin

$(SDK_LIBRARY): $(foreach lib,$(TARGET_LIBRARIES),lib$(lib).a)
	$(MAKELIB) $@ $^
	$(RANLIB) $@

sdk: sdk-dirs $(SDK_LIBRARY) $(SDK_HEADERS)
	@cp $(SDK_HEADERS) SDK/include
	@cp $(SDK_BINARIES) SDK/bin
	@$(STRIP) SDK/bin/*

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Rules.mak


