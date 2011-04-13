

ofxKinect
==================

ofxKinect is an Open Frameworks addon for the Xbox Kinect that runs on Linux and OS X.
OpenFrameworks is a cross platform open source toolkit for creative coding in C++.

[http://www.openframeworks.cc/](http://www.openframeworks.cc/)

Installation
------------

To use ofxKinect, first you need to download and install Open Frameworks. ofxKinect-beta.xcodeproj is developed against the latest version of Open Frameworks on github, while ofxKinect.xcodeproj will work with the 0062 release.

To get a copy of the repository you can download the source from [http://github.com/ofTheo/ofxKinect/zipball/master](http://github.com/ofTheo/ofxKinect/zipball/master) or, alternatively, you can use git clone:
<pre>
git clone git://github.com/ofTheo/ofxKinect.git
</pre>

The addon should sit in openFrameworks/addons/ofxKinect/.

#### Using the latest OpenFrameworks

If you want to work with the latest unstable (still in development) Open Frameworks, download the ofxKinect source from the experimental branch [https://github.com/ofTheo/ofxKinect/archives/experimental](https://github.com/ofTheo/ofxKinect/archives/experimental) or via git clone:
<pre>
git clone git://github.com/ofTheo/ofxKinect.git -b experimental
</pre> 

Warning: The experimental branch will be in flux, so don't be suprised if things do not always work as expected!

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

Windows support is currently in testing in the *develop* branch. The libfreenect Kinect drivers and an example Visual Studio 2010 solution are included.

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

On Mac, rename the project in XCode (do not rename the .xcodeproj file in Finder!): XCode Menu->Project->Rename

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
* add a search path to: `../../../addons/ofxKinect/libs/libusb/osx/libs` under Targets->YourApp->Build->Library Search Paths (make sure All Configurations and All Settings are selected)

Developing ofxKinect
--------------------

### Branch Layout

master is the latest stable version for 0062

develop is the unstable development branch for the current stable OF, ie 0062

experimental is the unstable development branch for the unstable OF, ie 007

This layout is designed so that a clone and the github page point to the stable version by default.

When adding new features or fixing bugs, you may need to create commits that should be merged across branches. Do this by creating a branch to do your work in, then merge this branch with other branches. Do NOT work directly on master.

Development for master is done in the develop branch and then merged. The experimental branch is for bleeding edge work with the latest unstable OF so that any new api changes do not break master or develop.

When a new stable version of OF is released, the current master is tagged with a version number (system to be determined) and experimental is merged into master.

This development model follows: http://nvie.com/posts/a-successful-git-branching-model/

### Git Help

#### Colored Output

Use the following commands to setup colored output form git which makes it easier to see changes:
<pre>
git config --global color.diff auto
git config --global color.status auto
git config --global color.branch auto
</pre>

#### Git Cheatsheet

Print the git help:
<pre>
git --help
</pre>

Print the help on a specific git tool:
<pre>
git help checkout
</pre>

Switching to a branch:
<pre>
git checkout branchname
</pre>

Show which branch you are currently working in:
<pre>
git branch -v
</pre>

Creating a new branch:
<pre>
git branch branchname
</pre>

Deleting a branch:
<pre>
git branch -d branchname
</pre>

Show the log of the last 10 commits:
<pre>
git log -10
</pre>

#### Resolving Merge Conflicts

See this [useful guide](http://book.git-scm.com/3_basic_branching_and_merging.html) on how to resolve merge conflicts.

### Developing

Checkout the develop branch:
<pre>
git git://github.com/ofTheo/ofxKinect.git -b develop
</pre>

Switch to the develop branch:
<pre>
git checkout develop
</pre>

Changes done in develop may need to be brought into master. This should not be taken lightly as master must always be stable and working with the latest stable version of OF.

#### The work flow is as follows:
* do your commits in develop
* switch to the master branch:
  <pre>
  git checkout master
  </pre>
* merge from branch to master:
  <pre>
  git merge develop
  </pre>
* if you have any conflicts, follow [this guide](http://book.git-scm.com/3_basic_branching_and_merging.html) on how to resolve merge commits
* push your changes to github:
  <pre>
  git push origin develop
  </pre>

### Experimental Development

The experimental branch is considered unstable and is for an upcoming release of OF. It will only be merged into master once the new version of OF becomes stable.

### Adding New Features Across Branches

If you want to add a feature to both develop and experimental, create a branch of develop, do your work, and merge the branch with both develop and experimetal. This avoid having to merge experimental with develop as any api changes between the 2 could break experimental.

#### The workflow is as follows:
* create a new branch for the feature from develop:
  <pre>
  git checkout -b newfeature develop`
  </pre>
* do your work, make commits, etc
* merge these changes to develop: 
  <pre>
  git checkout develop
  git merge --no-ff newfeature
  </pre>
* merge these changes to experimental:
  <pre>
  git checkout experimetal
  git merge --no-ff newfeature
  git push origin experimental
  </pre>
* close the feature branch:
  <pre>
  git checkout develop 
  git branch -d newfeature
  git push origin develop
  </pre>

#### Making Bugfixes Across Branches:

If you want to fix a bug found in master, do not do it directly, but through a branch which can then be merged across the main branches:

#### The workflow is as follows:
* create a new branch for the bugfix:
  <pre>
  git checkout -b newbugfix master
  </pre>
* do your work, make commits, etc
* merge these changes to master:
  <pre>
  git checkout master
  git merge --no-ff newbugfix
  </pre>
* merge these changes to develop:
  <pre>
  git checkout develop
  git merge --no-ff newbugfix
  git push origin develop
  </pre>
* merge these changes to experimental:
  <pre>
  git checkout experimental
  git merge --no-ff newbugfix
  git push origin experimental
  </pre>
* close the bugfix branch:
  <pre>
  git checkout master 
  git branch -d newbugfix
  git push origin master
  </pre> 

### Versioning

There is some talk about [Semantic Versioning](http://semver.org/) via git tagging but this will need to be discussed for the future.

