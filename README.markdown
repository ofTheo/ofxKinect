ofxKinect
==================

ofxKinect is an Open Frameworks addon for the Xbox Kinect that runs on Linux and OS X.
OpenFrameworks is a cross platform open source toolkit for creative coding in C++.

[http://www.openframeworks.cc/](http://www.openframeworks.cc/)

Installation
------------

To use ofxKinect, first you need to download and install [Open Frameworks](https://github.com/openframeworks/openFrameworks).

To get a copy of the repository you can download the source from [http://github.com/ofTheo/ofxKinect](http://github.com/ofTheo/ofxKinect/) or, alternatively, you can use git clone:
<pre>
git clone git://github.com/ofTheo/ofxKinect.git
</pre>

The addon should sit in `openFrameworks/addons/ofxKinect/`.

#### Which version to use?

If you are using a stable version (0062, 007, ...) of OpenFrameworks then you want to use a git tag of ofxKinect for that version. You can select the tag in the Github menu or clone and check it out using git.

For example, the following commands will clone ofxKinect and switch to the OF 0062 tagged version:
<pre>
git clone git://github.com/ofTheo/ofxKinect.git
git checkout 0062
</pre>

#### Using the latest ofxKinect

THe master branch of ofxKinect will work with the current master of OpenFrameworks and can be considered unstable. The development branch is used for testing new features and bugfixes before they are applied to master.

If you want to work with the latest unstable (still in development) ofxKinect, download the source from the development branch [https://github.com/ofTheo/ofxKinect/archives/experimental](https://github.com/ofTheo/ofxKinect/archives/development) or via git clone:
<pre>
git clone git://github.com/ofTheo/ofxKinect.git -b development
</pre> 

Warning: The development branch will be in flux, so don't be suprised if things do not always work as expected!

Running the Example Project
-------------------------------

The example projects are in the `example` folder.

#### OSX

Open the XCode project and hit "Build and Run". You might want to chosose "Release" instead of "Debug" for faster performance.

#### Linux

Open the Code::Blocks .cbp and hit F9 to build and run. Makefiles are also included.

You should create some udev rules in order to run the app without root privileges. As root write this to /etc/udev/rules.d/51-kinect.rules (this works on Ubuntu 10.10):
<pre>
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02ae", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02ad", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02b0", MODE="0660", GROUP="plugdev"
</pre>

#### Windows

libfreenect Kinect drivers and an example Visual Studio 2010 solution are included.

Make sure to install or update the libfreenect Kinect camera, motor, and audio drivers through Windows Device Manager by pointing it to the driver folder:
<pre>
libs/libusb/win/inf
</pre>

You may need to manually update each driver individually if you've plugged it in before. ofxKinect will not work if the drivers are not installed.

How to Create a New ofxKinect Project
-----------------------------------------

To develop your own project based on ofxKinect, simply copy the example project and rename it. You probably want to put it in your apps folder, for example, after copying:

`openFrameworks/addons/ofxKinect/example/ => openFrameworks/apps/myApps/example/`

Then after renaming:

`openFrameworks/apps/myApps/myKinectProject/`

On Mac, rename the project in XCode (do not rename the .xcodeproj file in Finder!): XCode Menu->Project->Rename (Xcode3) / Slow double click the project name and rename (Xcode4)

Adding ofxKinect to an Existing Project
---------------------------------------

If you want to add ofxKinect to another project, you need to make sure you include the src and libs folders:
<pre>
openFrameworks/addons/ofxKinect/src
openFrameworks/addons/ofxKinect/libs
</pre>

You will also need the ofxOpenCV addon which is included with OpenFrameworks. Add the following path to your header search paths:
`../../../addons/ofxOpenCV/libs/opencv/include`

For Xcode:
* create a new group "ofxKinect"
* drag these directories from ofxKinect into this new group: `ofxKinect/src` & `ofxKinect/libs`
* add a search path to: `../../../addons/ofxKinect/libs/libusb/osx/libs`
** Xcode3: under Targets->YourApp->Build->Library Search Paths (make sure All Configurations and All Settings are selected) and add the path
** Xcode4: add the lib path to your Project.xconfig, see the example

Developing ofxKinect
--------------------

Feel free to log bug reports and issues to the ofxKinect Github page: https://github.com/ofTheo/ofxKinect/issues

If you wish to help in ofxKinect dvelopment, please refer to the wiki: https://github.com/ofTheo/ofxKinect/wiki/Developing-ofxKinect

