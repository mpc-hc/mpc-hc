#!/bin/bash -e

HST=i686-pc-mingw32
TGT=x86_64-pc-mingw32
RT=root-$HST
PF=`pwd`/$RT
BD=`pwd`/build
DIRS="$PF $PF/$TGT build $BD/mingw $BD/mingw/build-$HST $BD/ffmpeg"
baseopts="--prefix=$PF --with-sysroot=$PF --target=$TGT"

updatemingw="false"

while opt=$1 && shift; do
  case "$opt" in
    "--help" )
      cat << EOF
    This build script will setup an entire buildroot environment consisting Ming64 and ffmpeg
    to build Mpc-hc x64 edition. Source code will be downloaded from SourceForge projects :
	- Crt and Headers from MingW x64 (http://sourceforge.net/project/showfiles.php?group_id=202880)
	- Ffmpeg from Mpc-hc (http://sourceforge.net/projects/mpc-hc/)

  To use this script, you should have :
	- Download MingW64 chaintool for i686-mingw platform 
	 (http://sourceforge.net/project/showfiles.php?group_id=202880&package_id=245516&release_id=546049)
	- Download MSys, and configure it with MingW64 with postinstall script 
	(http://sourceforge.net/project/showfiles.php?group_id=2435)
	- Start MSys and run this script
  link above).
	
      $0 [ --help ] [ --updatemingw ] 
      
    --help	Causes all other arguments to be ignored and results in the display of
		this text.

    --updatemingw 	Get latest of Ming64, and rebuild library 

EOF
    exit
    ;;

    "--updatemingw" )
      updatemingw="true"
      ;;

  esac
done

for i in $DIRS; do
[ -d $i ]  || mkdir $i && updatemingw="true"
done

if [[ $updatemingw == "true" ]]; then
	echo "Downloading Ming64 crt and headers.." && cd $BD/mingw
	svn -q co https://mingw-w64.svn.sourceforge.net/svnroot/mingw-w64/trunk .

	echo "Compiling crt.." && cd $BD/mingw/build-$HST
	../mingw-w64-crt/configure --prefix=$PF --with-sysroot=$PF --host=$TGT || exit 1
	make CFLAGS="-fno-leading-underscore -mno-cygwin" -s && make install || exit 1
	
	cp /mingw/lib/gcc/x86_64-pc-mingw32/ ../../../../../lib64
	cp $PF/x86_64-pc-mingw32/lib/libmingwex.a ../../../../../lib64
fi


echo "Downloading ffmpeg for Mpc-hc" && cd $BD/ffmpeg
svn -q co https://mpc-hc.svn.sourceforge.net/svnroot/mpc-hc/trunk/src/filters/transform/mpcvideodec/ffmpeg .

echo "Compiling ffmpeg for Mpc-hc..."
cd $BD/ffmpeg
make 64BIT=yes
