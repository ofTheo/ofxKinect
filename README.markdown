ofxKinect
=========

Copyright (c) 2010, 2011 ofxKinect Team

MIT License.

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "LICENSE.txt," in this distribution.

See https://github.com/ofTheo/ofxKinect for documentation as well as the [OF forums](http://forum.openframeworks.cc/index.php).

This project uses [libfreenect](https://github.com/OpenKinect/libfreenect), copyrighted by the Open Kinect Project using the Apache License v2. See the file "APACHE20" in libs/libfreenect.

Description
-----------

ofxKinect is an Open Frameworks addon for the Xbox Kinect that runs on Linux and OS X.
OpenFrameworks is a cross platform open source toolkit for creative coding in C++.

[http://www.openframeworks.cc/](http://www.openframeworks.cc/)

###NOTE: Does not currently support Kinect4Windows Hardware!

If you have a Kinect4Windows device, it will not currently work in ofxKinect as libfreenect does not support them yet. The [fix is in the works](https://github.com/OpenKinect/libfreenect/issues/298).

In the meantime, we suggest you use ofxOpenNI or get an Xbox Kinect instead ...

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
cd ofxKinect
git checkout 0062
</pre>

#### Using the latest ofxKinect

The master branch of ofxKinect will work with the current master of OpenFrameworks and can be considered unstable. The develop branch is used for testing new features and bugfixes before they are applied to master.

If you want to work with the latest unstable (still in development) ofxKinect, download the source from the develop branch [https://github.com/ofTheo/ofxKinect/tree/develop](https://github.com/ofTheo/ofxKinect/tree/develop) or via git clone:
<pre>
git clone git://github.com/ofTheo/ofxKinect.git -b develop
</pre> 

Warning: The develop branch will be in flux, so don't be suprised if things do not always work as expected!

Running the Example Project
---------------------------

The example projects are in the `example` folder.

### OSX

Xcode3: Open the Xcode project and hit "Build and Run". You might want to choose "Release" instead of "Debug" for faster performance.

Xcode4: Open the Xcode project, select the "ofxKinectExample" scheme, and hit "Run".

### Linux

Install the libusb-1.0 library. On Ubuntu, you can do this with:
<pre>
sudo apt-get install libusb-1.0-0-dev
</pre>

Open the Code::Blocks .cbp and hit F9 to build. Optionally, you can build the example with the Makefile.

To run it, use the terminal:
<pre>
make
cd bin
./example_debug
</pre>

Also, you should create some udev rules in order to run the app without root privileges. As the root user, write this to `/etc/udev/rules.d/51-kinect.rules` (this works on Ubuntu 10.10):
<pre>
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02ae", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02ad", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", SYSFS{idVendor}=="045e", SYSFS{idProduct}=="02b0", MODE="0660", GROUP="plugdev"
</pre>

From Ubuntu 12.10 the correct settings for udev rules are:
<pre>
SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ae", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ad", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02b0", MODE="0660", GROUP="plugdev"
</pre>


### Windows

Precompiled libfreenect Kinect drivers and an example Visual Studio 2010 solution as well as a Codeblocks workspace are included.

Make sure to install or update the libfreenect Kinect camera, motor, and audio drivers through Windows Device Manager by pointing it to the driver folder:
<pre>
libs/libusb/win/inf
</pre>

You may need to manually update each driver individually if you've plugged it in before. ofxKinect will not work if the drivers are not installed.

**NOTE**: You cannot use the OpenNI drivers and the libfreenect drivers included with ofxKinect at the same time. You must manually uninstall one and reinstall the other in the Device Manager. Sorry, that's just how it is. :P 

How to Create a New ofxKinect Project
-----------------------------------------

To develop your own project based on ofxKinect, simply copy the example project and rename it. You probably want to put it in your apps folder, for example, after copying:

`openFrameworks/addons/ofxKinect/example/ => openFrameworks/apps/myApps/example/`

Then after renaming:

`openFrameworks/apps/myApps/myKinectProject/`

### Mac (Xcode):

Rename the project in Xcode (do not rename the .xcodeproj file in Finder!): XCode Menu->Project->Rename (Xcode3) / Slow double click the project name and rename (Xcode4)

### Codeblocks (Win & Linux):

Close Codeblocks and rename the *.cbp and *.workspace files to the same name as the project folder. Open the workspace and readd the renamed project file.

### Windows (Visual Studio):

Close Visual Studio and rename the *.sln file. Open the solution, right click on the project in the project broswer and select "Rename ...".

Adding ofxKinect to an Existing Project
---------------------------------------

If you want to add ofxKinect to another project, you need to make sure you include the src folder and the :
<pre>
openFrameworks/addons/ofxKinect/src
</pre>
as well as search paths for the addons (ofxOpenCv) and libs (freenect, libusb) required by ofxKinect.

Don't forget to include the ofxOpenCv addon sources and oepncv libraries.

### Mac (Xcode):

In the Xcode project browser:

* add the ofxKinect sources to the project:
  * create a new group "ofxKinect"
  * drag these directories from ofxKinect into this new group: ofxKinect/src & ofxKinect/libs
* add a search path to the libusb headers: `../../../addons/ofxKinect/libs/libusb/include/libusb-1.0` to your project Target build settings
  * Xcode3: under Targets->YourApp->Build->Header Search Paths (make sure All Configurations and All Settings are selected) and add the path
  * Xcode4: add the lib path to your Project.xconfig, see the example
* add the path to the libusb precompiled library: `../../../addons/ofxKinect/libs/libusb/osx/libs/usb-1.0.a`
  * Xcode3: under Targets->YourApp->Build->Library Search Paths (make sure All Configurations and All Settings are selected) and add the path
  * Xcode4: add the lib path to your Project.xconfig, see the example
  
### Linux (Makefiles & Codeblocks):

Edit the Makefile config files:
 
* edit addons.make in your project folder and add the following line to the end of the file: 
	<pre>ofxKinect</pre>
* edit config.make in your project folder and change the lines for USER_LIBS to:
	<pre>
	USER_LIBS = -lusb-1.0
	</pre>

### Windows (VS & Codeblocks)

Add the project search paths

For libusb & freenect, link to the precompiled freenect library in:
<pre>
../../../addons/ofxKinect/libs/libfreenect/lib/vs2010/freenect.lib
</pre>

#### Windows (Visual Studio):

* add the ofxPd sources to the project:
	* right click on the addons folder in your project tree and create a new filter named "ofxKinect"
	* drag the src directory from the ofxKinect addon folder in Explorer into the ofxKinect folder in your project tree
* add header and library search paths:
	* right lick on your project and select "Properties"
	* choose "Debug" from the "Configurations" drop down box
	* under C/C++->General, add the following to the "Additional Include Directories":
	<pre>
	..\\..\\..\addons\ofxKinect\src
	..\\..\\..\addons\ofxPd\libs\libsfreenect
	</pre>
	* under Linker->General, add the following to the "Additional Library Directories":
	<pre>
	..\..\..\addons\ofxKinect\libs\libfreenect\lib\vs2010
	</pre>
	* repeat for the "Release" configuration
	
#### Windows (Codeblocks):

* add the ofxPd sources to the project:
	* right-click on your project in the project tree
	* select "Add Files Recursively ..."
	* navigate and choose the ofxKinect/src folder
* add search paths and libraries to link:
	* right-click on your project in the project tree
	* select "Build options..."
	* make sure the project name is selected in the tree (not release or debug)
	* select the "Search diectories" tab, click add the search paths:
	<pre>
	..\\..\\..\addons\ofxKinect\src
	..\\..\\..\addons\ofxPd\libs\libsfreenect
	</pre>
	* select the "Linker settings" tab, add the following to Link libraries:
	<pre>
	m
	pthread
	</pre>
	* select the "Linker settings" tab, add the following to Other liker options:
	<pre>
	../../../addons/ofxKinect/libs/libfreenect/lib/vs2010/freenect.lib
	</pre>

Notes
-----

### Using multiple kinects

ofxKinect supports multiple kinects, however stability is based on the bandwidth of your usb hardware. If you only require the depth image, it is recommended to disable the video grabbing of the rgb/ir images from your devices:
<pre>
kinect.init(false, false);  // disable video image (faster fps)
</pre>

For Kinect4Windows, Microsoft states that only 2 Kinects can be supported on the same USB bus. In practice on OSX, this proves to be the case as, even with the RGB images disabled, there are transfer errors using ofxKinect and 3 Kinects simultaneously. If you need to support many Kinects, you will probably need to add extra USB controllers to your machine …

Developing ofxKinect
--------------------

Feel free to log bug reports and issues to the ofxKinect Github page: https://github.com/ofTheo/ofxKinect/issues

If you wish to help in ofxKinect dvelopment, please refer to the wiki: https://github.com/ofTheo/ofxKinect/wiki/Developing-ofxKinect

