##########################################################################
#
#    Core target
#
#    (c) 2001-2002 Gilles Boccon-Gibod
#    Author: Gilles Boccon-Gibod (bok@bok.net)
#
##########################################################################

##########################################################################
# source files
##########################################################################
CORE_SOURCES = 								\
	Ap4Atom.cpp 							\
	Ap4AtomFactory.cpp						\
	Ap4ContainerAtom.cpp					\
	Ap4FrmaAtom.cpp							\
	Ap4FtypAtom.cpp							\
	Ap4HdlrAtom.cpp							\
	Ap4MdhdAtom.cpp 						\
	Ap4MoovAtom.cpp 						\
	Ap4MvhdAtom.cpp 						\
	Ap4SmhdAtom.cpp                         \
	Ap4NmhdAtom.cpp                         \
	Ap4HmhdAtom.cpp                         \
	Ap4VmhdAtom.cpp                         \
	Ap4SchmAtom.cpp							\
	Ap4StcoAtom.cpp 						\
	Ap4StscAtom.cpp 						\
	Ap4StsdAtom.cpp 						\
	Ap4StssAtom.cpp							\
	Ap4StszAtom.cpp 						\
	Ap4SttsAtom.cpp							\
	Ap4TkhdAtom.cpp 						\
	Ap4IsmaCryp.cpp                         \
	Ap4IsfmAtom.cpp                         \
	Ap4IkmsAtom.cpp                         \
	Ap4TimsAtom.cpp                         \
	Ap4TrakAtom.cpp 						\
	Ap4SdpAtom.cpp                          \
	Ap4RtpAtom.cpp                          \
	Ap4UrlAtom.cpp 							\
	Ap4CttsAtom.cpp							\
	Ap4DrefAtom.cpp							\
	Ap4EsdsAtom.cpp							\
	Ap4Descriptor.cpp						\
	Ap4DescriptorFactory.cpp 				\
	Ap4SLConfigDescriptor.cpp		        \
	Ap4UnknownDescriptor.cpp		        \
	Ap4DecoderConfigDescriptor.cpp  		\
	Ap4DecoderSpecificInfoDescriptor.cpp	\
	Ap4EsDescriptor.cpp 	    			\
	Ap4TrefTypeAtom.cpp						\
	Ap4File.cpp 							\
	Ap4Track.cpp 							\
	Ap4Utils.cpp							\
	Ap4Movie.cpp							\
	Ap4RtpHint.cpp  						\
	Ap4Sample.cpp							\
	Ap4SampleTable.cpp                      \
	Ap4AtomSampleTable.cpp			        \
	Ap4SyntheticSampleTable.cpp             \
	Ap4SampleDescription.cpp	            \
	Ap4SampleEntry.cpp					    \
	Ap4FileWriter.cpp                       \
	Ap4HintTrackReader.cpp					\
	Ap4Processor.cpp                        \
	Ap4ByteStream.cpp 						\
	Ap4DataBuffer.cpp						\
	Ap4Debug.cpp 							\

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Crypto.exp
include $(BUILD_ROOT)/Makefiles/Core.exp

##########################################################################
# targets
##########################################################################
TARGET_SOURCES = $(CORE_SOURCES)

libAP4_Core.a: $(patsubst %.cpp,%.o,$(TARGET_SOURCES))

##########################################################################
# make path
##########################################################################
VPATH += $(SOURCE_ROOT)/Core

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Rules.mak
