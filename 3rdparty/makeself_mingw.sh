MINGWHOME=/opt/mingw/x86_64-darwin

set -e

export PATH=${MINGWHOME}/bin:${PATH}

export target=x86_64-w64-mingw32
export CC=${target}-gcc
export CFLAGS=-O2
export CXX=${target}-g++
export CXXFLAGS=-O2
export AR=${target}-ar
export RM="rm -f"
export MAKE="make -f makefile.mingw"

${MAKE} self GRDIR=./

mkdir -p build
cp -p lib/gks/demo.exe build/gksdemo.exe
cp -p lib/gks/libgks.lib build/
cp -p lib/gks/libgks.dll build/
cp -r lib/gks/fonts build/
cp -p lib/gks/plugin/*.dll build/
cp -p lib/gr/demo.exe build/grdemo.exe
cp -p lib/gr/libgr.lib build/
cp -p lib/gr/libgr.dll build/
cp -p ${MINGWHOME}/x86_64-w64-mingw32/lib/libgcc_s_seh-1.dll build/
cp -p ${MINGWHOME}/x86_64-w64-mingw32/lib/libstdc++-6.dll build/

for dir in lib 3rdparty
do
  find -E ./${dir} -regex '.*\.(a|lib|dll|exe)' -exec rm {} \;
done
