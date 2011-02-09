ofxKinect
==================

ofxKinect is an Open Frameworks addon for the Xbox Kinect that runs on Linux and OS X.
OpenFrameworks is a cross platform open source toolkit for creative coding in C++.

[http://www.openframeworks.cc/](http://www.openframeworks.cc/)

Installation
------------

To use ofxKinect, first you need to download and install Open Frameworks. ofxKinect-beta.xcodeproj is developed against the latest version of Open Frameworks on github, while ofxKinect.xcodeproj will work with the 0062 release.

To get a copy of the repository you can download the source from [http://github.com/ofTheo/ofxKinect/zipball/master](http://github.com/ofTheo/ofxKinect/zipball/master)or, alternatively, you can use git clone. The addon should sit in openFrameworks/addons/ofxKinect/.

Running the Example Project
-------------------------------

If you're using OS X, open the XCode project in ofxKinect/example/ and hit "Build and Run". You might want to chosoe "Release" instead of "Debug" for faster performance.

If you're using Linux, you should open the Code::Blocks .cbp and hit F9 to build and run.

You should create some udev rules in order to run the app without root privileges. As root write this to /etc/udev/rules.d/51-kinect.rules (this works on Ubuntu 10.10):
<pre>
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02ae", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02ad", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02b0", MODE="0660", GROUP="plugdev"
</pre>

Sorry, there is currently no Windows version. :( Please pitch in and help us.

How to Create a New ofxKinect Project
-----------------------------------------

To develop your own project based on ofxKinect, simply copy the example project and rename it. You probably want to put it in your apps folder, for example, after copying:

`openFrameworks/addons/ofxKinect/example/ => openFrameworks/apps/myApps/example/`

Then after renaming:

`openFrameworks/apps/myApps/myKinectProject/`

On Mac, rename the project in XCode (do not just rename the .xcodeproj fiel in Finder): Project->Rename

Adding ofxKinect to an Existing Project
---------------------------------------

If you want to add ofxKinect to another project, you need to make sure you include the src and libs folders:

<pre>
openFrameworks/addons/ofxKinect/src
openFrameworks/addons/ofxKinect/libs
</pre>

For XCode:

* create a new group "ofxKinect"
* drag these directories from ofxKinect into this new group: ofxKinect/src & ofxKinect/libs
* add a search path to: `../../../addons/ofxOpenNI/libs/libusb/osx/libs` under Targets->YourApp->Build->Library Search Paths (make sure All Configurations and All Settings are selected)

