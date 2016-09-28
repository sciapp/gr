sudo apt-get -y update
#sudo apt-get -y dist-upgrade
sudo apt-get -y install make mingw-w64 pkg-config xorg-dev g++ gcc cmake

export PREFIX=x86_64-w64-mingw32
export ARCHITECTURE=x86_64
export OS=w64_amd64-cross-mingw32
export CC=${PREFIX}-gcc
export CXX=${PREFIX}-g++
export AR=${PREFIX}-ar
export RM="rm -f"

export MAKE="make -f makefile.mingw"
${MAKE} self GRDIR=./

mkdir artifacts
cp lib/gks/demo.exe artifacts/gksdemo.exe
cp lib/gks/libgks.lib artifacts/
cp lib/gks/libgks.dll artifacts/
cp lib/gks/libgks.a artifacts/
cp -r lib/gks/fonts artifacts/
cp lib/gks/plugin/*.dll artifacts/
cp lib/gks/plugin/*.a artifacts/
cp lib/gr/demo.exe artifacts/grdemo.exe
cp lib/gr/libgr.lib artifacts/
cp lib/gr/libgr.dll artifacts/
cp lib/gr/libgr.a artifacts/
cp lib/gr3/libGR3.dll artifacts/
cp lib/gr3/libGR3.a artifacts/
cp /usr/lib/gcc/x86_64-w64-mingw32/4.9-win32/libgcc_s_seh-1.dll artifacts/

