// Avisynth v1.0 beta.  Copyright 2000 Ben Rudiak-Gould.
// http://www.math.berkeley.edu/~benrg/avisynth.html

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .


#pragma once

#ifdef _MSC_VER
  #include <crtdbg.h>
#else
  #define _ASSERTE(x) assert(x)
  #include <assert.h>
#endif


enum { AVISYNTH_INTERFACE_VERSION = 1 };


// I had problems with Premiere wanting 1-byte alignment for its structures,
// so I now set the Avisynth struct alignment explicitly here.
#pragma pack(push,8)


// The VideoInfo struct holds global information about a clip (i.e.
// information that does not depend on the frame number).  The GetVideoInfo
// method in IClip returns this struct.

struct VideoInfo {
  int width, height;    // width=0 means no video
  unsigned fps_numerator, fps_denominator;
  int num_frames;
  enum { UNKNOWN=0, BGR24=0x13, BGR32=0x14, YUY2=0x22 };
  unsigned char pixel_type;
  bool field_based;

  int audio_samples_per_second;   // 0 means no audio
  int num_audio_samples;
  bool stereo, sixteen_bit;

  // useful functions of the above
  bool HasVideo() const { return !!width; }
  bool HasAudio() const { return !!audio_samples_per_second; }
  bool IsRGB() const { return !!(pixel_type&0x10); }
  bool IsRGB24() const { return pixel_type == BGR24; }
  bool IsRGB32() const { return pixel_type == BGR32; }
  bool IsYUV() const { return !!(pixel_type&0x20); }
  bool IsYUY2() const { return pixel_type == YUY2; }
  int BytesFromPixels(int pixels) const { return pixels * (pixel_type&7); }
  int RowSize() const { return BytesFromPixels(width); }
  int BitsPerPixel() const { return (pixel_type&7) * 8; }
  int BMPSize() const { return height * ((RowSize()+3) & -4); }
  int AudioSamplesFromFrames(int frames) const { return int(__int64(frames) * audio_samples_per_second * fps_denominator / fps_numerator); }
  int FramesFromAudioSamples(int samples) const { return int(__int64(samples) * fps_numerator / fps_denominator / audio_samples_per_second); }
  int AudioSamplesFromBytes(int bytes) const { return bytes >> (stereo + sixteen_bit); }
  int BytesFromAudioSamples(int samples) const { return samples << (stereo + sixteen_bit); }
  int BytesPerAudioSample() const { return BytesFromAudioSamples(1); }

  // useful mutator
  void SetFPS(unsigned numerator, unsigned denominator) {
    unsigned x=numerator, y=denominator;
    while (y) {   // find gcd
      unsigned t = x%y; x = y; y = t;
    }
    fps_numerator = numerator/x;
    fps_denominator = denominator/x;
  }
};


// VideoFrameBuffer holds information about a memory block which is used
// for video data.  For efficiency, instances of this class are not deleted
// when the refcount reaches zero; instead they're stored in a linked list
// to be reused.  The instances are deleted when the corresponding AVS
// file is closed.

class VideoFrameBuffer {
  unsigned char* const data;
  const int data_size;
  // sequence_number is incremented every time the buffer is changed, so
  // that stale views can tell they're no longer valid.
  long sequence_number;

  friend class VideoFrame;
  friend class Cache;
  long refcount;

public:
  VideoFrameBuffer(int size);
  VideoFrameBuffer();
  ~VideoFrameBuffer();

  const unsigned char* GetReadPtr() const { return data; }
  unsigned char* GetWritePtr() { ++sequence_number; return data; }
  int GetDataSize() { return data_size; }
  int GetSequenceNumber() { return sequence_number; }
  int GetRefcount() { return refcount; }
};


class IClip;
class PClip;
class PVideoFrame;
class IScriptEnvironment;
class AVSValue;


// VideoFrame holds a "window" into a VideoFrameBuffer.  Operator new
// is overloaded to recycle class instances.

class VideoFrame {
  int refcount;
  VideoFrameBuffer* const vfb;
  const int offset, pitch, row_size, height;

  friend class PVideoFrame;
  void AddRef() { ++refcount; }
  void Release() { if (refcount==1) --vfb->refcount; --refcount; }

  friend class ScriptEnvironment;
  friend class Cache;

  VideoFrame(VideoFrameBuffer* _vfb, int _offset, int _pitch, int _row_size, int _height);

  void* operator new(unsigned size);

public:
  int GetPitch() const { return pitch; }
  int GetRowSize() const { return row_size; }
  int GetHeight() const { return height; }

  // generally you shouldn't use these two 
  VideoFrameBuffer* GetFrameBuffer() const { return vfb; }
  int GetOffset() const { return offset; }

  // in plugins use env->SubFrame()
  VideoFrame* Subframe(int rel_offset, int new_pitch, int new_row_size, int new_height) const;

  const unsigned char* GetReadPtr() const { return vfb->GetReadPtr() + offset; }

  bool IsWritable() const { return (refcount == 1 && vfb->refcount == 1); }

  unsigned char* GetWritePtr() const {
    return IsWritable() ? (vfb->GetWritePtr() + offset) : 0;
  }

  ~VideoFrame() { --vfb->refcount; }
};


// Base class for all filters.
class IClip {
  friend class PClip;
  friend class AVSValue;
  int refcnt;
  void AddRef() { ++refcnt; }
  void Release() { if (!--refcnt) delete this; }
public:
  IClip() : refcnt(0) {}

  virtual int __stdcall GetVersion() { return AVISYNTH_INTERFACE_VERSION; }

  virtual PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) = 0;
  virtual bool __stdcall GetParity(int n) = 0;  // return field parity if field_based, else parity of first field in frame
  virtual void __stdcall GetAudio(void* buf, int start, int count, IScriptEnvironment* env) = 0;  // start and count are in samples
  virtual const VideoInfo& __stdcall GetVideoInfo() = 0;
  virtual __stdcall ~IClip() {}
};


// smart pointer to IClip
class PClip {

  IClip* p;

  IClip* GetPointerWithAddRef() const { if (p) p->AddRef(); return p; }
  friend class AVSValue;
  friend class VideoFrame;

  void Init(IClip* x) {
    if (x) x->AddRef();
    p=x;
  }
  void Set(IClip* x) {
    if (x) x->AddRef();
    if (p) p->Release();
    p=x;
  }

public:
  PClip() { p = 0; }
  PClip(const PClip& x) { Init(x.p); }
  PClip(IClip* x) { Init(x); }
  void operator=(IClip* x) { Set(x); }
  void operator=(const PClip& x) { Set(x.p); }

  IClip* operator->() const { return p; }

  // useful in conditional expressions
  operator void*() const { return p; }
  bool operator!() const { return !p; }

  ~PClip() { if (p) p->Release(); }
};


// smart pointer to VideoFrame
class PVideoFrame {

  VideoFrame* p;

  void Init(VideoFrame* x) {
    if (x) x->AddRef();
    p=x;
  }
  void Set(VideoFrame* x) {
    if (x) x->AddRef();
    if (p) p->Release();
    p=x;
  }

public:
  PVideoFrame() { p = 0; }
  PVideoFrame(const PVideoFrame& x) { Init(x.p); }
  PVideoFrame(VideoFrame* x) { Init(x); }
  void operator=(VideoFrame* x) { Set(x); }
  void operator=(const PVideoFrame& x) { Set(x.p); }

  VideoFrame* operator->() const { return p; }

  // for conditional expressions
  operator void*() const { return p; }
  bool operator!() const { return !p; }

  ~PVideoFrame() { if (p) p->Release(); }
};


class AVSValue {
public:

  AVSValue() { type = 'v'; }
  AVSValue(IClip* c) { type = 'c'; clip = c; if (c) c->AddRef(); }
  AVSValue(const PClip& c) { type = 'c'; clip = c.GetPointerWithAddRef(); }
  AVSValue(bool b) { type = 'b'; boolean = b; }
  AVSValue(int i) { type = 'i'; integer = i; }
  AVSValue(float f) { type = 'f'; floating_pt = f; }
  AVSValue(double f) { type = 'f'; floating_pt = float(f); }
  AVSValue(const char* s) { type = 's'; string = s; }
  AVSValue(const AVSValue* a, int size) { type = 'a'; array = a; array_size = size; }
  AVSValue(const AVSValue& v) { Assign(&v, true); }

  ~AVSValue() { if (IsClip() && clip) clip->Release(); }
  AVSValue& operator=(const AVSValue& v) { Assign(&v, false); return *this; }

  // Note that we transparently allow 'int' to be treated as 'float'.
  // There are no int<->bool conversions, though.

  bool Defined() const { return type != 'v'; }
  bool IsClip() const { return type == 'c'; }
  bool IsBool() const { return type == 'b'; }
  bool IsInt() const { return type == 'i'; }
  bool IsFloat() const { return type == 'f' || type == 'i'; }
  bool IsString() const { return type == 's'; }
  bool IsArray() const { return type == 'a'; }

  PClip AsClip() const { _ASSERTE(IsClip()); return IsClip()?clip:0; }
  bool AsBool() const { _ASSERTE(IsBool()); return boolean; }
  int AsInt() const { _ASSERTE(IsInt()); return integer; }
  const char* AsString() const { _ASSERTE(IsString()); return IsString()?string:0; }
  double AsFloat() const { _ASSERTE(IsFloat()); return IsInt()?integer:floating_pt; }

  bool AsBool(bool def) const { _ASSERTE(IsBool()||!Defined()); return IsBool() ? boolean : def; }
  int AsInt(int def) const { _ASSERTE(IsInt()||!Defined()); return IsInt() ? integer : def; }
  double AsFloat(double def) const { _ASSERTE(IsFloat()||!Defined()); return IsInt() ? integer : type=='f' ? floating_pt : def; }
  const char* AsString(const char* def) const { _ASSERTE(IsString()||!Defined()); return IsString() ? string : def; }

  int ArraySize() const { _ASSERTE(IsArray()); return IsArray()?array_size:1; }
  const AVSValue& operator[](int index) const {
    _ASSERTE(IsArray() && index>=0 && index<array_size);
    return (IsArray() && index>=0 && index<array_size) ? array[index] : *this;
  }

private:

  short type;  // 'a'rray, 'c'lip, 'b'ool, 'i'nt, 'f'loat, 's'tring, or 'v'oid
  short array_size;
  union {
    IClip* clip;
    bool boolean;
    int integer;
    float floating_pt;
    const char* string;
    const AVSValue* array;
  };

  void Assign(const AVSValue* src, bool init) {
    if (src->IsClip() && src->clip)
      src->clip->AddRef();
    if (!init && IsClip() && clip)
      clip->Release();
    // make sure this copies the whole struct!
    ((__int32*)this)[0] = ((__int32*)src)[0];
    ((__int32*)this)[1] = ((__int32*)src)[1];
  }
};


// instantiable null filter
class GenericVideoFilter : public IClip {
protected:
  PClip child;
  VideoInfo vi;
public:
  GenericVideoFilter(PClip _child) : child(_child) { vi = child->GetVideoInfo(); }
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return child->GetFrame(n, env); }
  void __stdcall GetAudio(void* buf, int start, int count, IScriptEnvironment* env) { child->GetAudio(buf, start, count, env); }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  bool __stdcall GetParity(int n) { return child->GetParity(n); }
};


class AvisynthError /* exception */ {
public:
  const char* const msg;
  AvisynthError(const char* _msg) : msg(_msg) {}
};


// For GetCPUFlags.  These are the same as in VirtualDub.
enum {
  CPUF_FORCE			= 0x01,
  CPUF_FPU			= 0x02,
  CPUF_MMX			= 0x04,
  CPUF_INTEGER_SSE	= 0x08,		// Athlon MMX extensions or Intel SSE
  CPUF_SSE			= 0x10,		// Full SSE (PIII)
  CPUF_SSE2			= 0x20,		// (PIV)
  CPUF_3DNOW			= 0x40,
  CPUF_3DNOW_EXT		= 0x80,		// Athlon 3DNow! extensions
};


class IScriptEnvironment {
public:
  virtual __stdcall ~IScriptEnvironment() {}

  virtual /*static*/ long __stdcall GetCPUFlags() = 0;

  virtual char* __stdcall SaveString(const char* s, int length = -1) = 0;
  virtual char* __stdcall Sprintf(const char* fmt, ...) = 0;
  // note: val is really a va_list; I hope everyone typedefs va_list to a pointer
  virtual char* __stdcall VSprintf(const char* fmt, void* val) = 0;

  __declspec(noreturn) virtual void __stdcall ThrowError(const char* fmt, ...) = 0;

  class NotFound /*exception*/ {};  // thrown by Invoke and GetVar

  typedef AVSValue (__cdecl *ApplyFunc)(AVSValue args, void* user_data, IScriptEnvironment* env);

  virtual void __stdcall AddFunction(const char* name, const char* params, ApplyFunc apply, void* user_data) = 0;
  virtual bool __stdcall FunctionExists(const char* name) = 0;
  virtual AVSValue __stdcall Invoke(const char* name, const AVSValue args, const char** arg_names=0) = 0;

  virtual AVSValue __stdcall GetVar(const char* name) = 0;
  virtual bool __stdcall SetVar(const char* name, const AVSValue& val) = 0;
  virtual bool __stdcall SetGlobalVar(const char* name, const AVSValue& val) = 0;

  virtual void __stdcall PushContext(int level=0) = 0;
  virtual void __stdcall PopContext() = 0;

  // align should be 4 or 8
  virtual PVideoFrame __stdcall NewVideoFrame(const VideoInfo& vi, int align=8) = 0;

  virtual bool __stdcall MakeWritable(PVideoFrame* pvf) = 0;

  virtual /*static*/ void __stdcall BitBlt(unsigned char* dstp, int dst_pitch, const unsigned char* srcp, int src_pitch, int row_size, int height) = 0;

  typedef void (__cdecl *ShutdownFunc)(void* user_data, IScriptEnvironment* env);
  virtual void __stdcall AtExit(ShutdownFunc function, void* user_data) = 0;

  virtual void __stdcall CheckVersion(int version = AVISYNTH_INTERFACE_VERSION) = 0;

  virtual PVideoFrame __stdcall Subframe(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size, int new_height) = 0;
};


// avisynth.dll exports this; it's a way to use it as a library, without
// writing an AVS script or without going through AVIFile.
IScriptEnvironment* __stdcall CreateScriptEnvironment(int version = AVISYNTH_INTERFACE_VERSION);


#pragma pack(pop)

