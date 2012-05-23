#! /bin/bash

WD=$(dirname $0)

###

cd $WD

# get latest source
git clone git://github.com/OpenKinect/libfreenect.git

# remove uneeded makefiles
rm libfreenect/src/CMakeLists.txt

# we dont need freenect audio (yet)
rm libfreenect/src/audio.c
rm libfreenect/src/libfreenect-audio.h
rm libfreenect/src/loader.c
#rm libfreenect/src/loader.h

# no python either
rm libfreenect/src/fwfetcher.py

# make folder
mkdir -p ../libs/libfreenect

# copy licenses, etc
cp -v libfreenect/APACHE20 ../libs/libfreenect
cp -v libfreenect/CONTRIB ../libs/libfreenect
cp -v libfreenect/GPL2 ../libs/libfreenect

# copy sources
cp -Rv libfreenect/src/* ../libs/libfreenect

# cleanup
rm -rfv libfreenect
