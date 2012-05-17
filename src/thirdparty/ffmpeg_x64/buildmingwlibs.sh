#!/bin/bash -e

CC=x86_64-w64-mingw32-gcc
HST=i686-pc-mingw32
TGT=x86_64-w64-mingw32
RT=root-$HST
PF=`pwd`/$RT
BD=`pwd`/build
GCCVER=`$CC -v 2>&1 | tail -1 | awk '{print $3}'`

updatemingw="false"

while opt=$1 && shift
do
  case "$opt" in
    "--help" )
      cat << EOF

If you have followed the instructions on the wiki page then you got everything
already installed.

This build script will set up an entire building environment consisting of
MinGW64 to build the needed mingw libs for mpc-hc64. The source code will be
downloaded from the MinGW64 SourceForge project:
  (http://sourceforge.net/project/showfiles.php?group_id=202880)

To use this script, you should first:
  - Download MinGW64 chaintool for the i686-mingw platform
    (http://sourceforge.net/project/showfiles.php?group_id=202880&package_id=245516&release_id=546049)
  - Download MSYS and configure MingW64 with the postinstall script
    (or edit your "MSYS\etc\fstab" file and add your mingw path)
    (http://sourceforge.net/project/showfiles.php?group_id=2435)
  - Patch MSYS with latest release of CoreUtils and Bash
  - Install Subversion command line client from http://subversion.tigris.org
    and add it to a folder which is in your PATH
  - Start MSYS and run this script

    $0 [ --help ] [ --update ] [ --compile ]

  --help          Displays this text

  --update        Gets the latest MinGW64

  --compile       Starts MinGW64 compilation
EOF
    exit
    ;;

  "--update" )
    updatemingw="true"
    compilewmingw="false"
    ;;

  "--compile" )
    compilewmingw="true"
    ;;

  esac
done

for i in "$PF" "$PF/$TGT" "build" "$BD/mingw" "$BD/mingw/build-$HST"
do
  [ -d "$i" ] || mkdir "$i" || updatemingw="true"
done

if [[ $updatemingw == "true" ]]
then
  echo "Downloading MinGW64 crt and headers..."
  cd "$BD/mingw"

  # remove patched files
  if [ -f mingw-w64-crt/misc/delayimp.c ]
  then
  rm mingw-w64-crt/misc/delayimp.c
  fi

  svn -q co https://mingw-w64.svn.sourceforge.net/svnroot/mingw-w64/stable/v2.x .

  # apply Mingw64 compatibility patch
  patch -p0 -i ../../mpchc_Mingw64.patch

  dest="$PF/$TGT/include"
  [ -d "$dest" ] && echo "$dest" already exists || ( cp -prf mingw-w64-headers/include "$dest" && /bin/find "$dest" -name ".svn" | xargs rm -rf )
fi

if [[ $compilewmingw == "true" ]]
then
  echo "Compiling MinGW64 crt and headers..."
  cd "$BD/mingw/build-$HST"
  ../mingw-w64-crt/configure --prefix="$PF" --with-sysroot="$PF" --host="$TGT" --disable-lib32 || exit 1
  make -j4 CFLAGS="-O2 -fno-leading-underscore -pipe" -s && make install || exit 1
  cp "/mingw/lib/gcc/x86_64-w64-mingw32/$GCCVER/libgcc.a" "$BD/../../../../../../lib64/libgcc.a"
  cp "$PF/x86_64-w64-mingw32/lib/libmingwex.a" "$BD/../../../../../../lib64/libmingwex.a"
fi
