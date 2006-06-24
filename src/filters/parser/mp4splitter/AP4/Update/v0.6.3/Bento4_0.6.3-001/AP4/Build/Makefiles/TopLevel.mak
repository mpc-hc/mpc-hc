##########################################################################
#
#    top level make rules and variables
#
#    (c) 2001-2002 Gilles Boccon-Gibod
#    Author: Gilles Boccon-Gibod (bok@bok.net)
#
##########################################################################

##########################################################################
# exported variables
##########################################################################
export BUILD_ROOT
export SOURCE_ROOT
export TARGET

export FILE_BYTE_STREAM_IMPLEMENTATION

export CC
export AUTODEP_CPP
export AUTODEP_STDOUT
export ARCHIVE
export COMPILE_CPP
export LINK_CPP
export MAKELIB
export MAKESHAREDLIB
export RANLIB
export STRIP
export DEBUG_CPP
export OPTIMIZE_CPP
export PROFILE_CPP
export DEFINES_CPP
export WARNINGS_CPP
export INCLUDES_CPP
export LIBRARIES_CPP

##########################################################################
# modular targets
##########################################################################

# ------- Setup -------------
.PHONY: Setup
Setup:
	mkdir $(OUTPUT_DIR)
    
# ------- Core -------------
Core: Crypto
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Core.mak

# ------- System -----------
System:
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/System.mak

# ------- Codecs -----------
Codecs:
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Codecs.mak

# ------- Crypto -----------
Crypto:
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Crypto.mak

# ------- Apps -----------
ALL_APPS = mp4dump mp4info mp42aac aac2mp4 mp4decrypt mp4encrypt mp4edit mp4extract mp4rtphintinfo
export ALL_APPS
Apps: $(ALL_APPS)

##################################################################
# cleanup
##################################################################
TO_CLEAN += *.d *.o *.a *.exe $(ALL_APPS) SDK

##################################################################
# end targets
##################################################################
sdk: Core System Codecs Crypto Apps
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/SDK.mak

mp4dump: Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4Dump.mak

mp4info: Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4Info.mak

mp42aac: Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp42Aac.mak

aac2mp4: Codecs Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Aac2Mp4.mak

mp4decrypt: Crypto Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4Decrypt.mak

mp4encrypt: Crypto Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4Encrypt.mak

mp4edit: Crypto Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4Edit.mak

mp4extract: Crypto Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4Extract.mak

mp4rtphintinfo: Crypto Core System
	$(TITLE)
	@$(INVOKE_SUBMAKE) -f $(BUILD_ROOT)/Makefiles/Mp4RtpHintInfo.mak

##################################################################
# includes
##################################################################
include $(BUILD_ROOT)/Makefiles/Rules.mak






