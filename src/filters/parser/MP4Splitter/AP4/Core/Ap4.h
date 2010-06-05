/*****************************************************************
|
|    AP4 - Main Header
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/
/** 
* @file
* @brief Top Level include file
*
* Applications should only need to include that file, as it will
* include all the more specific include files required to use the API
*/

/** @mainpage Bento4 SDK
*
* @section intro_sec Introduction
* Bento4/AP4 is a C++ class library designed to read and write ISO-MP4 files.
* This format is defined in ISO/IEC 14496-12, 14496-14 and 14496-15. 
* The format is a derivative of the Apple Quicktime file format. 
* Because of that, Bento4 can be used to read and write a number of Quicktime files 
* as well, even though  some Quicktime specific features are not supported.
* In addition, Bento4 supports a number of extensions as defined in various
* other specifications. This includes some support for ISMA Encrytion and
* Decryption as defined in the ISMA E&A specification (http://www.isma.tv), 
* OMA 2.0 PDCF Encryption and Decryption as defined in the OMA 2.0 PDCF
* specification (http://www.openmobilealliance.org) and iTunes compatible
* metadata.
* The SDK includes a number of command line tools, built using the class library,
* that serve as general purpose tools as well as examples of how to use the API.
* 
* The SDK is designed to be cross-platform. The code is very portable; it can
* be compiled with any sufficiently modern C++ compiler. The code does not rely
* on any external library; all the code necessary to compile the SDK and its
* tools is included in the standard distribution. The standard distribution 
* contains makefiles for unix-like operating systems, including Linux, project
* files for Microsoft Visual Studio, and an XCode project for  MacOS X. There is
* also support for building the library with the SCons build system.
*
* @section building Building the SDK
* Building the SDK will produce a C++ class library and some command line tools.
* For the makefile-based configurations, 'make sdk' will produce an SDK
* directory layout with a bin/ lib/ and include/ subdirectories containing
* respectively the binaries, library and header files.
*
* @subsection build_win32 Windows
* Open the solution file Build/Targets/Build/Targets/x86-microsoft-win32-vs2005
* Building the solution will build the class library and command line tools.
* The script Build/Targets/Build/Targets/x86-microsoft-win32-vs2005/make-sdk.sh
* will create the SDK directory structure as described above.
*
* @subsection build_linux Linux
* Go to Build/Targets/x86-unknown-linux or Build/Targets/<target-name> 
* for any linux-based target, and use the 'make'
* command to build the library and tools, or 'make sdk' to build the SDK 
* directory structure.
*
* @subsection build_cygwin Cygwin
* Go to Build/Targets/x86-unknown-cygwin and follow the same instructions as
* for the Linux platform.
*
* @subsection build_macos MacOS
* Use the XCode project file located under Build/Targets/ppc-apple-macosx
*
* @subsection build_scons Using SCons
* There is experimental support for building the SDK using the python-based
* SCons tool (http://www.scons.org). The top level configuration is located 
* at the root of the SDK, and will output all the object files and binaries
* to a sub-directory under Build/SCons/Targets/<target-name>/<config-name>.
*
* @subsection build_others Other Platforms
* Other plaftorms can be built by adapting the makefiles for the generic
* gcc-based configurations. 
*
* @section mp4_structure Structure of an MP4 file
*
* An MP4 file consists of a tree of atoms (also known as boxes). The atoms
* contain the information about the different media tracks for the file, as
* well as other information, such as metadata.
* The Bento4 class library parses files an constructs an in-memory representation 
* of the atoms, and provides an API that is an abstraction layer for the
* way the information is actually encoded in those atoms.
*
* @section reading Reading Files
*
* The class #AP4_File represents all the information about an MP4 file.
* Internally, a tree of #AP4_Atom objects plus other helper objects holds 
* the actual information.
* To create an instance of the class, the caller must pass a reference to 
* an #AP4_ByteStream object that represents the file data storage. The SDK
* includes two subclasses of the abstract #AP4_ByteStream class: 
* #AP4_FileByteStream for reading/writing disk-based files and
* #AP4_MemoryByteStream for working with in-memory file images.
* Once you have created an #AP4_File object, you can get to the media data by
* accessing the #AP4_Track objects of its #AP4_Movie (see #AP4_File::GetMovie).
* The #AP4_Track exposes the necessary methods for you to get the
* #AP4_SampleDescription (typically to initialize your decoder) and to get the 
* #AP4_Sample objects from the track.
* These #AP4_Sample objects give you the meta information you need (such as
* timestamps) as well as the sample data that they point to.
* You can also explore the entire tree of atoms by calling #AP4_File::Inspect,
* passing an instance of #AP4_AtomInspector. #AP4_AtomInspector is an abstract
* base class for a visitor of the tree of #AP4_Atom objects. The SDK includes
* an concrete subclass, #AP4_PrintInspector, can be used to print out a text
* representation of the atoms as they are visited. See the Mp4Dump command line
* application for an example. 
* 
* @section writing Writing Files
*
* To create a new MP4 file, you first create an #AP4_SyntheticSampleTable
* sample table for each track in your file. You specify the sample description
* for the media samples in each track by calling #AP4_SyntheticSampleTable::AddSampleDescription
* with the description for the samples of that track. Samples can then be added
* to the sample table with the #AP4_SyntheticSampleTable::AddSample method. 
* Once all the samples of all the tracks have been added to the sample tables, 
* you can create an #AP4_Movie, and an #AP4_Track from each sample table, add the track
* to the movie, and finally create an #AP4_File from the movie object.
* Finally, the #AP4_File object can be serialized to a byte stream (such as an
* #AP4_FileByteStream) using the #AP4_FileWriter class.
* See the Aac2Mp4 application for an example.
*
* @section tracks Tracks
* 
* The #AP4_Movie object of an #AP4_File has a list of #AP4_Track objects. Each
* #AP4_Track represents a media track in the file. From this object, you can 
* obtain the type, duration, etc.. of the tracks. This object also provides
* access to the media samples in the track. Tracks are made up of media samples,
* stored in an 'mdat' atom (media data) and a sample table that provides all the
* information about the individual samples, such as size, timestamps, sample
* description, etc... Media samples are represented by #AP4_Sample objects. 
* You can obtain a track's samples by calling the #AP4_Track::GetSample method. 
* You can also directly read the payload of a media sample by calling the
* #AP4_Track::ReadSample method if you want to bypass the intermediate step of
* going through an #AP4_Sample object.
* The information about the samples in a track is represented by subclasses of
* the #AP4_SampleDescription class. The sample descriptions contain information 
* about media-specific parameters, such as video resolution, audio sampling rates
* and others.
* See the Mp4Info and Mp42Aac command line applications as examples.
*
* @section advanced Advanced Topics
*
* @subsection factory Custom Atoms
*
* The SDK has built-in support for most of the standard atom types as defined
* in the spec. But an application may want to extend the library to support 
* non-standard atom type, such as custom atoms whose definition is proprietary.
* The base class for all atoms is #AP4_Atom. Instances of subclasses of this class
* are created by the #AP4_AtomFactory object. The factory knows about all the 
* #AP4_Atom subclasses included in the SDK, and maintains a list of 
* #AP4_AtomFactory::TypeHandler type handlers. When the factory encounters an atom
* that is not one of the built-in atom type, it calls all the registered type
* handlers, in turn, until one of them accept to handle the type and create the 
* corresponding atom. The custom atoms must be implemented as subclasses of 
* #AP4_Atom and override at least the #AP4_Atom::WriteFields method. The custom
* atoms should also override #AP4_Atom::InspectFields if they want to provide
* meaningful information when inspected.
*
* @subsection tranformations Transformations
* The SDK provides support for transforming MP4 files. Transformations are useful
* to perform tasks such as editing (removing or adding atoms) and encryption or
* decryption. When the atom tree for the file changes, the entire file needs to 
* change, because in most cases, the size of the 'moov' atom will change, and
* thus change the offset of the media samples in the 'mdat' atom.
* To facilitate this operation, the class #AP4_Processor provides the base
* functionality for managing all the updates to the sample tables. Subclasses
* of the #AP4_Processor class can override certain methods to carry out specific
* changes to the structure of the atom tree and/or the media samples. #AP4_Processor
* defines an abstract base class, #AP4_Processor::TrackHandler that defines the
* interface that specific track modification classes must implement. A subclass
* of #AP4_Processor may create such an #AP4_Processor::TrackHandler subclass instance
* when is #AP4_Processor::CreateTrackHandler is called for one of the tracks in the
* file.
* See the sample applications Mp4Edit, Mp4Encrpt and Mp4Decrypt as examples. 
*
* @subsection encryption Encryption and Decryption
*
* The SDK has support for encrypting and decrypting tracks as specified by the
* ISMA Encryption and Authentication specification as well as OMA 2.0 and 2.1 DCF and PDCF. 
* The supporting classes found in Ap4IsmaCryp.h and AP4_OmaDcf.h provide a subclass
* of #AP4_Processor for encrypting or decrypting entire files. 
* The class #AP4_IsmaCipher and #AP4_OmaDcfSampleDecrypter implement the generic
* #AP4_SampleDecrypter interface that provides support for decrypting individual samples.
* The parameters necessary to instantiate an #AP4_IsmaCipher or an #AP4_OmaDcfSampleDecrypter 
* can be retrieved from an encrypted track by accessing the track's sample descriptions, 
* which are instances of the #AP4_ProtectedSampleDescription class. A more general factory
* method, #AP4_SampleDecrypter::Create can be called, passing an instance of
* #AP4_ProtectedSampleDescription and the cipher key, and the correct concrete class will
* automatically be instanciated based on the type of encryption that was used for the track.
*
* @subsection RTP Packets
*
* For files that contain hint tracks, the SDK provides support for generating
* RTP packets that can be used to stream the media using the RTP and RTSP protocols.
* See the application Mp4RtpHintInfo for an example of how to generate the create 
* RTP packets from a file and generate the SDP information for the RTSP protocol.
*/

#ifndef _AP4_H_
#define _AP4_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Config.h"
#include "Ap4Version.h"
#include "Ap4Types.h"
#include "Ap4Constants.h"
#include "Ap4Results.h"
#include "Ap4Debug.h"
#include "Ap4Utils.h"
#include "Ap4DynamicCast.h"
#include "Ap4FileByteStream.h"
#include "Ap4Movie.h"
#include "Ap4Track.h"
#include "Ap4File.h"
#include "Ap4FileWriter.h"
#include "Ap4FileCopier.h"
#include "Ap4HintTrackReader.h"
#include "Ap4Processor.h"
#include "Ap4MetaData.h"
#include "Ap4AtomFactory.h"
#include "Ap4SampleEntry.h"
#include "Ap4Sample.h"
#include "Ap4DataBuffer.h"
#include "Ap4SampleTable.h"
#include "Ap4SyntheticSampleTable.h"
#include "Ap4AtomSampleTable.h"
#include "Ap4FragmentSampleTable.h"
#include "Ap4UrlAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4MvhdAtom.h"
#include "Ap4MehdAtom.h"
#include "Ap4TrakAtom.h"
#include "Ap4TrexAtom.h"
#include "Ap4HdlrAtom.h"
#include "Ap4DrefAtom.h"
#include "Ap4TkhdAtom.h"
#include "Ap4MdhdAtom.h"
#include "Ap4StsdAtom.h"
#include "Ap4StscAtom.h"
#include "Ap4StcoAtom.h"
#include "Ap4StszAtom.h"
#include "Ap4EsdsAtom.h"
#include "Ap4SttsAtom.h"
#include "Ap4CttsAtom.h"
#include "Ap4StssAtom.h"
#include "Ap4FtypAtom.h"
#include "Ap4VmhdAtom.h"
#include "Ap4SmhdAtom.h"
#include "Ap4NmhdAtom.h"
#include "Ap4HmhdAtom.h"
#include "Ap4SchmAtom.h"
#include "Ap4FrmaAtom.h"
#include "Ap4TimsAtom.h"
#include "Ap4RtpAtom.h"
#include "Ap4SdpAtom.h"
#include "Ap4IkmsAtom.h"
#include "Ap4IsfmAtom.h"
#include "Ap4IsltAtom.h"
#include "Ap4TrefTypeAtom.h"
#include "Ap4OmaDcf.h"
#include "Ap4IsmaCryp.h"
#include "Ap4OdafAtom.h"
#include "Ap4OhdrAtom.h"
#include "Ap4OdheAtom.h"
#include "Ap4OddaAtom.h"
#include "Ap4AvccAtom.h"
#include "Ap4Marlin.h"
#include "Ap4GrpiAtom.h"
#include "Ap48bdlAtom.h"
#include "Ap4MovieFragment.h"
#include "Ap4LinearReader.h"
#include "Ap4TfhdAtom.h"
#include "Ap4SampleSource.h"
#include "Ap4Mpeg2Ts.h"
#include "Ap4Piff.h"

#endif // _AP4_H_
