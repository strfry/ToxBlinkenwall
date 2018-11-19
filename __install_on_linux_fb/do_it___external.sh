#! /bin/bash



echo "starting ..."

START_TIME=$SECONDS

## ----------------------
full="1"
O_OPTIONS=" -O3 "
opus_sys=1
vpx_sys=1
x264_sys=1
ffmpeg_sys=1
numcpus_=4
quiet_=1
## ----------------------


_HOME2_=$(dirname $0)
export _HOME2_
_HOME_=$(cd $_HOME2_;pwd)
export _HOME_

export qqq=""

if [ "$quiet_""x" == "1x" ]; then
	export qqq=" -qq "
fi


redirect_cmd() {
    if [ "$quiet_""x" == "1x" ]; then
        "$@" > /dev/null 2>&1
    else
        "$@"
    fi
}

echo "cleanup ..."
rm -Rf /script/build/
rm -Rf /script/inst/

echo "installing system packages ..."


syslibs_str__="_"

if [ $opus_sys == 1 ]; then
    syslibs_str__="$syslibs_str__""o"
fi
if [ $vpx_sys == 1 ]; then
    syslibs_str__="$syslibs_str__""v"
fi
if [ $x264_sys == 1 ]; then
    syslibs_str__="$syslibs_str__""x"
fi
if [ $ffmpeg_sys == 1 ]; then
    syslibs_str__="$syslibs_str__""f"
fi

echo "with system libs for: $syslibs_str__"



echo "installing more system packages ..."

apk add wget git cmake ffmpeg-dev x264-dev automake gcc v4l-utils-dev \
	libsodium-dev make libc-dev linux-headers \
	autoconf automake libtool

# cmake3 ?
type -a cmake
cmake --version

cmake_version=$(cmake --version|grep 'make .ersion'|sed -e 's#.make .ersion ##'|tr -d " ")
cmake_version_major=$(echo $cmake_version|cut -d"." -f 1|tr -d " ")
cmake_version_minor=$(echo $cmake_version|cut -d"." -f 2|tr -d " ")
need_newer_cmake=0
if [ "$cmake_version_major""x" == "2x" ]; then
    need_newer_cmake=1
fi

if [ "$cmake_version_major""x" == "3x" ]; then
    if [ "$cmake_version_minor""x" == "0x" ]; then
        need_newer_cmake=1
    fi
    if [ "$cmake_version_minor""x" == "1x" ]; then
        need_newer_cmake=1
    fi
    if [ "$cmake_version_minor""x" == "2x" ]; then
        need_newer_cmake=1
    fi
fi

if [ "$need_newer_cmake""x" == "1x" ]; then
    redirect_cmd apt-get install $qqq -y --force-yes build-essential
    mkdir -p $_HOME_/build
    cd $_HOME_/build/
    mkdir cmake3
    cd cmake3
    wget http://www.cmake.org/files/v3.5/cmake-3.5.2.tar.gz
    tar xf cmake-3.5.2.tar.gz
    cd cmake-3.5.2
    redirect_cmd ./configure --prefix=/usr
    redirect_cmd make -j"$numcpus_"
    redirect_cmd make install
fi

type -a cmake
cmake --version



# echo $_HOME_
cd $_HOME_
mkdir -p build

export _SRC_=$_HOME_/build/
export _INST_=$_HOME_/inst/

# echo $_SRC_
# echo $_INST_

mkdir -p $_SRC_
mkdir -p $_INST_

export PKG_CONFIG_PATH=$_INST_/lib/pkgconfig



if [ "$full""x" == "1x" ]; then


rm -Rf $_INST_

echo "NASM ..."

apk add nasm

#nasm
cd $_HOME_/build


echo "filteraudio ..."

cd $_HOME_/build
rm -Rf filter_audio
git clone https://github.com/irungentoo/filter_audio.git > /dev/null 2>&1
cd filter_audio
export DESTDIR=$_INST_
export PREFIX=""
redirect_cmd make
make install > /dev/null 2>&1
export DESTDIR=""
unset DESTDIR
export PREFIX=""
unset PREFIX


fi



echo "c-toxcore ..."


cd $_HOME_/build


rm -Rf c-toxcore
git clone https://github.com/Zoxcore/c-toxcore c-toxcore > /dev/null 2>&1
cd c-toxcore
git checkout toxav-multi-codec
./autogen.sh > /dev/null 2>&1
export CFLAGS=" -D_GNU_SOURCE -g -O3 -I$_INST_/include/ -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable "
export LDFLAGS=" -O3 -L$_INST_/lib "
redirect_cmd ./configure \
  --prefix=$_INST_ \
  --disable-soname-versions --disable-testing --disable-shared
unset CFLAGS
unset LDFLAGS

redirect_cmd make -j"$numcpus_" || exit 1


make install > /dev/null 2>&1




echo "compiling ToxBlinkenwall ..."

apk add libjpeg-turbo-dev alsa-lib-dev

cd $_HOME_/build

rm -Rf tbw_build
mkdir -p tbw_build
cd tbw_build/

git clone https://github.com/zoff99/ToxBlinkenwall xx > /dev/null 2>&1
mv xx/* . > /dev/null 2>&1
mv xx/.??* . > /dev/null 2>&1
rm -Rf xx > /dev/null 2>&1

cd toxblinkenwall

sed -i -e 's#define HAVE_OUTPUT_OPENGL .*#define HAVE_FRAMEBUFFER 1#' toxblinkenwall.c

gcc -g -O3 toxblinkenwall.c rb.c ringbuf.c \
-I$_INST_/include/ -L$_INST_/lib -fPIC \
-l:libtoxcore.a -l:libtoxav.a -l:libtoxencryptsave.a -l:libsodium.a \
-o toxblinkenwall \
-lasound -lm -lpthread -l:libopus.a -l:libvpx.a -l:libavcodec.a \
-l:libx264.a -l:libavutil.a -l:libv4lconvert.a -lrt -l:libjpeg.a


echo "###############"
echo "###############"
echo "###############"

pwd

ELAPSED_TIME=$(($SECONDS - $START_TIME))

echo "compile time: $(($ELAPSED_TIME/60)) min $(($ELAPSED_TIME%60)) sec"


ls -hal toxblinkenwall && cp -av toxblinkenwall /artefacts/toxblinkenwall

# so files can be accessed outside of docker
chmod -R a+rw /script/
chmod -R a+rw /artefacts/
