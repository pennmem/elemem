#! /bin/bash
set -euo pipefail

# Handle Command Line Arguments
while getopts "ht" ARG; do
  case "$ARG" in
    h) echo "USAGE: $(basename $0) -h, -t"
       exit 1 ;;
    t) TEST_SYSTEM=true ;; # CerebusSim and CereStimStub
    :) echo "argument missing" >&2 ;;
    \?) echo "Something is wrong" >&2 ;;
  esac
done
shift "$((OPTIND-1))"

# Prereqs
brew install cmake qt@5 PkgConfig fftw
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
QT_VERSION=$(brew list --versions qt@5 | awk '{ print $2 }')
QT_PATH="$(brew --prefix)/Cellar/Qt@5/$QT_VERSION"
echo "export CMAKE_PREFIX_PATH=$QT_PATH" >> ~/.zshrc
source ~/.zshrc

# Build and Install Qt (not used)
#curl https://qt.mirror.constant.com/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz --output qt-everywhere-src-5.12.12.tar.xz
#tar -xzvf qt-everywhere-src-5.12.12.tar.xz
#cd qt-everywhere-src-5.12.12
#git clone https://github.com/qt/qt5.git
#git checkout v5.12.11
#./init-repository
#git submodule update --init --recursive
#./configure -prefix $PWD/qtbase -release -nomake tests -nomake examples -opensource -confirm-license QMAKE_APPLE_DEVICE_ARCHS=arm64
## Add the below code line below to qtbase/src/plugins/platforms/cocoa/qiosurfacegraphicsbuffer.h
## under the line #include <private/qcore_mac_p.h>
## #include "../../../../include/QtCore/../../src/corelib/io/qdebug.h"
#make -j4

# Setup Configs
mkdir -p ~/Desktop/ElememConfigs
cp config/* ~/Desktop/ElememConfigs

# Build Elemem
mkdir -p build; cd "$_"
FLAGS=""
if [ "$OSTYPE" != "win32" ] || [ "$TEST_SYSTEM" = true ]; then
  FLAGS="${FLAGS} -DCEREBUS_HW=OFF -DCERESTIM_STUB=ON"
  perl -i -pe's/"eeg_system": "Cerebus"/"eeg_system": "CerebusSim"/g' ../dist/sys_config.json
  #perl -i -pe's/"stim_system": "CereStim"/"stim_system": "CereStimSim"/g' ../dist/sys_config.json
  perl -i -pe's/-Dunix=1/-Dunix=1 -Wno-deprecated-declarations/g' ../CMakeLists.txt
fi
cmake $FLAGS ..
make -j
cd ..

# Build StimProc
cd stimproc
mkdir -p build; cd "$_"
cmake ..
make -j
cd ../..
