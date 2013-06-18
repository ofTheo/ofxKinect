#! /bin/bash

WD=$(dirname $0)

DEST=../libs/libfreenect

###

cd $WD

# get latest source
#git clone git://github.com/OpenKinect/libfreenect.git
#git clone git://github.com/wizgrav/libfreenect.git
git clone git://github.com/danomatika/libfreenect.git

# remove uneeded makefiles
rm libfreenect/src/CMakeLists.txt
rm libfreenect/src/libfreenect.pc.in

# we dont need freenect audio
rm libfreenect/include/libfreenect-audio.h
rm libfreenect/src/audio.c
rm libfreenect/src/loader.c
rm libfreenect/src/loader.h

# no python either
rm libfreenect/src/fwfetcher.py

# make folders
mkdir -p $DEST/include
mkdir -p $DEST/src

# copy licenses, etc
cp -v libfreenect/APACHE20 $DEST
cp -v libfreenect/CONTRIB $DEST
cp -v libfreenect/GPL2 $DEST

# copy headers
cp -Rv libfreenect/include/* $DEST/include

# copy sources
cp -Rv libfreenect/src/* $DEST/src

# cleanup
rm -rf libfreenect
