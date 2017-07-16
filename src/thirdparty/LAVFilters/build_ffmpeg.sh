#!/bin/sh

echo "$(pwd)" | grep -q '[[:blank:]]' &&
  echo "Out of tree builds are impossible with whitespace in source path." && exit 1

bin_folder=bin

if [ "${1}" == "x64" ]; then
  arch=x86_64
  archdir=x64
  cross_prefix=x86_64-w64-mingw32-
  lav_folder=LAVFilters64
  mpc_hc_folder=mpc-hc_x64
else
  arch=x86
  archdir=Win32
  cross_prefix=
  lav_folder=LAVFilters
  mpc_hc_folder=mpc-hc_x86
fi

if [ "${2}" == "Debug" ]; then
  FFMPEG_DLL_PATH=$(readlink -f ../../..)/${bin_folder}/${mpc_hc_folder}_Debug/${lav_folder}
  BASEDIR=$(pwd)/src/bin_${archdir}d
else
  FFMPEG_DLL_PATH=$(readlink -f ../../..)/${bin_folder}/${mpc_hc_folder}/${lav_folder}
  BASEDIR=$(pwd)/src/bin_${archdir}
fi

THIRDPARTYPREFIX=${BASEDIR}/thirdparty
FFMPEG_BUILD_PATH=${THIRDPARTYPREFIX}/ffmpeg
FFMPEG_LIB_PATH=${BASEDIR}/lib

make_dirs() {
  mkdir -p ${FFMPEG_LIB_PATH}
  mkdir -p ${FFMPEG_BUILD_PATH}
  mkdir -p ${FFMPEG_DLL_PATH}
}

copy_libs() {
  # install -s --strip-program=${cross_prefix}strip lib*/*-lav-*.dll ${FFMPEG_DLL_PATH}
  cp lib*/*-lav-*.dll ${FFMPEG_DLL_PATH}
  ${cross_prefix}strip ${FFMPEG_DLL_PATH}/*-lav-*.dll
  cp -u lib*/*.lib ${FFMPEG_LIB_PATH}
}

clean() {
  cd ${FFMPEG_BUILD_PATH}
  echo Cleaning...
  if [ -f config.mak ]; then
    make distclean > /dev/null 2>&1
  fi
  cd ${BASEDIR}
}

configure() {
  OPTIONS="
    --enable-shared                 \
    --disable-static                \
    --enable-gpl                    \
    --enable-version3               \
    --enable-w32threads             \
    --disable-demuxer=matroska      \
    --disable-filters               \
    --enable-filter=scale,yadif,w3fdif \
    --disable-protocol=async,cache,concat,httpproxy,icecast,md5,subfile \
    --disable-muxers                \
    --enable-muxer=spdif            \
    --disable-bsfs                  \
    --enable-bsf=extract_extradata,vp9_superframe \
    --disable-hwaccels              \
    --enable-hwaccel=h264_dxva2     \
    --enable-hwaccel=hevc_dxva2     \
    --enable-hwaccel=vc1_dxva2      \
    --enable-hwaccel=wmv3_dxva2     \
    --enable-hwaccel=mpeg2_dxva2    \
    --enable-hwaccel=vp9_dxva2      \
    --enable-libspeex               \
    --enable-libopencore-amrnb      \
    --enable-libopencore-amrwb      \
    --enable-avresample             \
    --enable-avisynth               \
    --disable-avdevice              \
    --disable-postproc              \
    --disable-swresample            \
    --disable-encoders              \
    --disable-devices               \
    --disable-programs              \
    --disable-debug                 \
    --disable-doc                   \
    --build-suffix=-lav             \
    --arch=${arch}"

  EXTRA_CFLAGS="-fno-tree-vectorize -D_WIN32_WINNT=0x0502 -DWINVER=0x0502 -I../../../thirdparty/include"
  EXTRA_LDFLAGS=""
  if [ "${arch}" == "x86_64" ]; then
    OPTIONS="${OPTIONS} --enable-cross-compile --cross-prefix=${cross_prefix} --target-os=mingw32 --pkg-config=pkg-config"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L../../../thirdparty/lib64"
  else
    OPTIONS="${OPTIONS} --cpu=i686"
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -mmmx -msse -msse2 -mfpmath=sse -mstackrealign"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L../../../thirdparty/lib32"
  fi

  sh ../../../ffmpeg/configure --extra-ldflags="${EXTRA_LDFLAGS}" --extra-cflags="${EXTRA_CFLAGS}" ${OPTIONS}
}

build() {
  echo Building...
  make -j$NUMBER_OF_PROCESSORS 2>&1 | tee make.log
  ## Check the return status and the log to detect possible errors
  [ ${PIPESTATUS[0]} -eq 0 ] && ! grep -q -F "rerun configure" make.log
}

configureAndBuild() {
  cd ${FFMPEG_BUILD_PATH}
  ## Don't run configure again if it was previously run
  if [ ../../../ffmpeg/configure -ot config.mak ] &&
     [ ../../../../build_ffmpeg.sh -ot config.mak ]; then
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
    copy_libs
    CONFIGRETVAL=$?
  fi
  cd ${BASEDIR}
}

echo Building ffmpeg in GCC ${arch} Release config...

make_dirs

CONFIGRETVAL=0

if [ "${3}" == "Clean" ]; then
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

exit ${CONFIGRETVAL}
