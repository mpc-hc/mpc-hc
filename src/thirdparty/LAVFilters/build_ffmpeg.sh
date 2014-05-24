#!/bin/sh

if [ "${1}" == "x64" ]; then
  arch=x86_64
  archdir=x64
else
  arch=x86
  archdir=Win32
fi

make_dirs() {
  if [ ! -d bin_${archdir}/lib ]; then
    mkdir -p bin_${archdir}/lib
  fi

  if [ ! -d bin_${archdir}d/lib ]; then
    mkdir -p bin_${archdir}d/lib
  fi
}

strip_libs() {
  if [ "${arch}" == "x86_64" ]; then
    x86_64-w64-mingw32-strip lib*/*-lav-*.dll
  else
    strip lib*/*-lav-*.dll
  fi
}

copy_libs() {
  cp lib*/*-lav-*.dll ../../bin_${archdir}
  cp lib*/*.lib ../../bin_${archdir}/lib
  cp lib*/*-lav-*.dll ../../bin_${archdir}d
  cp lib*/*.lib ../../bin_${archdir}d/lib
}

clean() {
  echo Cleaning...
  if [ -f config.mak ]; then
    make distclean > /dev/null 2>&1
  fi
}

configure() {
  OPTIONS="
    --enable-shared                 \
    --disable-static                \
    --enable-version3               \
    --enable-w32threads             \
    --disable-demuxer=matroska      \
    --disable-decoder=opus          \
    --disable-parser=opus           \
    --disable-filters               \
    --enable-filter=yadif           \
    --enable-filter=scale           \
    --disable-protocols             \
    --enable-protocol=file          \
    --enable-protocol=pipe          \
    --enable-protocol=mmsh          \
    --enable-protocol=mmst          \
    --enable-protocol=rtp           \
    --enable-protocol=http          \
    --disable-muxers                \
    --enable-muxer=spdif            \
    --disable-hwaccels              \
    --enable-hwaccel=h264_dxva2     \
    --enable-hwaccel=vc1_dxva2      \
    --enable-hwaccel=wmv3_dxva2     \
    --enable-hwaccel=mpeg2_dxva2    \
    --enable-libspeex               \
    --enable-libopencore-amrnb      \
    --enable-libopencore-amrwb      \
    --enable-libopus                \
    --enable-avresample             \
    --enable-avisynth               \
    --disable-avdevice              \
    --disable-postproc              \
    --disable-swresample            \
    --disable-encoders              \
    --disable-bsfs                  \
    --disable-devices               \
    --disable-programs              \
    --disable-debug                 \
    --disable-doc                   \
    --build-suffix=-lav             \
    --arch=${arch}"

  EXTRA_CFLAGS="-D_WIN32_WINNT=0x0502 -DWINVER=0x0502 -I../../thirdparty/include"
  EXTRA_LDFLAGS=""
  if [ "${arch}" == "x86_64" ]; then
    OPTIONS="${OPTIONS} --enable-cross-compile --cross-prefix=x86_64-w64-mingw32- --target-os=mingw32"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L../../thirdparty/lib64"
  else
    OPTIONS="${OPTIONS} --cpu=i686"
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -mmmx -msse -mfpmath=sse"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L../../thirdparty/lib32"
  fi

  sh ../../ffmpeg/configure --extra-ldflags="${EXTRA_LDFLAGS}" --extra-cflags="${EXTRA_CFLAGS}" ${OPTIONS}
}

build() {
  echo Building...
  make -j8 2>&1 | tee make.log
  ## Check the return status and the log to detect possible errors
  [ ${PIPESTATUS[0]} -eq 0 ] && ! grep -q -F "rerun configure" make.log
}

configureAndBuild() {
  ## Don't run configure again if it was previously run
  if [ -f config.mak ]; then
    echo Skipping configure...
  else
    echo Configuring...

    ## run configure, redirect to file because of a msys bug
    configure > config.out 2>&1
    CONFIGRETVAL=$?

    ## show configure output
    cat config.out
  fi

  ## Only if configure succeeded, actually build
  if [ ${CONFIGRETVAL} -eq 0 ]; then
    build &&
    strip_libs &&
    copy_libs
    CONFIGRETVAL=$?
  fi
}

echo Building ffmpeg in GCC ${arch} Release config...

cd src

make_dirs

out_dir=bin_${archdir}/ffmpeg
if [ ! -d ${out_dir} ]; then
  mkdir -p ${out_dir}
fi
cd ${out_dir}

CONFIGRETVAL=0

if [ "${2}" == "Clean" ]; then
  clean
  CONFIGRETVAL=$?
else
  ## Check if configure was previously run
  if [ -f config.mak ]; then
    CLEANBUILD=0
  else
    CLEANBUILD=1
  fi

  configureAndBuild

  ## In case of error and only if we didn't start with a clean build,
  ## we try to rebuild from scratch including a full reconfigure
  if [ ! ${CONFIGRETVAL} -eq 0 ] && [ ${CLEANBUILD} -eq 0 ]; then
    echo Trying again with forced reconfigure...
    clean && configureAndBuild
  fi
fi

cd ../../..

exit ${CONFIGRETVAL}
