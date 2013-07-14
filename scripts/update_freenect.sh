#! /bin/bash

WD=$(dirname $0)

DEST=../libs/libfreenect

###

cd $WD

# get latest source
#git clone git://github.com/OpenKinect/libfreenect.git
git clone git://github.com/wizgrav/libfreenect.git
#git clone git://github.com/danomatika/libfreenect.git

# remove uneeded makefiles
rm libfreenect/src/CMakeLists.txt
rm libfreenect/src/libfreenect.pc.in

# we dont need freenect audio
rm libfreenect/include/libfreenect-audio.h
rm libfreenect/src/audio.c
rm libfreenect/src/loader.c

# no python either
rm libfreenect/src/fwfetcher.py

# make folders
mkdir -p $DEST/include
mkdir -p $DEST/src
mkdir -p $DEST/src/vs2010
mkdir -p $DEST/src/win_cb

# copy licenses, etc
cp -v libfreenect/APACHE20 $DEST
cp -v libfreenect/CONTRIB $DEST
cp -v libfreenect/GPL2 $DEST

# copy headers
cp -Rv libfreenect/include/* $DEST/include

# copy sources
cp -Rv libfreenect/src/* $DEST/src

# rename .c files to .cpp so VS2010 compiles them with something newer than C89!
mv $DEST/src/cameras.c $DEST/src/cameras.cpp
mv $DEST/src/core.c $DEST/src/core.cpp
mv $DEST/src/registration.c $DEST/src/registration.cpp
mv $DEST/src/tilt.c $DEST/src/tilt.cpp
mv $DEST/src/usb_libusb10.c $DEST/src/usb_libusb10.cpp

# copy windows specfic stuff
cp -Rv libfreenect/platform/windows $DEST/src/vs2010
cp -Rv libfreenect/platform/windows $DEST/src/win_cb

# cleanup
rm -rf libfreenect
