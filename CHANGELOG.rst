^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package libsegwayrmp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.2.10 (2013-12-10)
-------------------
* The segwayrmp_gui is now installed so it is available from binaries

0.2.9 (2013-06-11)
------------------
* Export -ldl on Linux
  Fix a bug where the explicit link against -ldl
  is not exported by the find_package and
  pkg-config files, which causes down stream
  projects to fail to compile.

0.2.8 (2013-05-18)
------------------
* Update find_package infrastructure
  * rename to libsegwayrmpConfig.cmake
  * template in the install path for searching
  * Fix logic around _FOUND
  * Search for ftd2xx on Apple also
  * use libsegwayrmp prefix instead of segwayrmp

0.2.7 (2013-05-03)
------------------
* Added Piyush Khandelwal to maintainer list for bloom release updates.
* Added pkgconfig file generation for linux. This also introduces the need to have a version number inside CMake apart from package.xml. (fixes `#8 <https://github.com/segwayrmp/libsegwayrmp/issues/8>`_)
* Fixed libftd2xx installation issue on linux. Tested using a direct cmake installation and catkin_make_isolated. Seems to be working fine on 64 bit. (Fixes `#7 <https://github.com/segwayrmp/libsegwayrmp/issues/7>`_)

0.2.6 (2013-04-18)
------------------
* Add missing run_depend on catkin and fix missing link against pthread

0.2.5 (2013-04-16 13:59)
------------------------
* Another missing import of iostream

0.2.4 (2013-04-16 12:10)
------------------------
* Adding missing include of iostream

0.2.3 (2013-01-20 20:30)
------------------------
* Fixed a build bug

0.2.2 (2013-01-20 17:03)
------------------------
* Add buildtool depend on cmake

0.2.1 (2013-01-17 17:12)
------------------------
* Add boost as a dependency
* Update .travis.yml

0.2.0 (2013-01-17 14:12)
------------------------
* update copyright
* Readme
* Linking bug with Boost System
* Adding travis support
* Adding GUI dependencies to the package.xml
* Preparation for release into groovy as 3rdparty
* Removed ROS specific code
* Merge pull request `#4 <https://github.com/segwayrmp/libsegwayrmp/issues/4>`_ from utexas-bwi/master
  reset all integrators before starting read.
* reset all integrators before starting read. Prevents the first couple of values from being arbitrary if the integrators are to be reset
* Updates to the manifest and build system to assist building of dependent packages in ROS.
* Fixing a compile problem on Linux
* Inverting commands sent to the segway from the GUI so it turns correctly
* Added joystick support, and other improvements to the GUI
* Added the moveCounts function which allows you to command the base using counts rather than approximated velocities
* Cleaning up build system
* Added a cancel function to rmp_io to fix a bug where the library would hang on disconnect if the device was shutdown before disconnecting
* Moved the enumerateUSBDevices function out of the ftd2xx_rmp_io class so it could be used by the GUI.
* Disable the copy constructors.
* Adding Graphical User Interface which is useful for configuring the Segway and validating the SegwayRMP library.
* Removing sublime-workspace file
* Adding .cc extension files and other missing files.
* Modified the connect method to no longer explicitly set the operational mode and controller gains as these are saved between segway restarts and should be a one time configuration or explicit mode request.  Additionally added a `shutdown` command which sends the previously unimplemented "RMP shutdown message".  This involved making the rmp_io class respect the packet.id value.
* Changed the way the Serial dependency is searched for.  If it is not found with `find_package` and the `ROS_ROOT` environment variable is set then it tries to find the serial library using rosbuild.
* Ignore CMakeLists.txt.user from Qt Creator.
* Tested working in tractor mode.
* Fixing a serial cmake typo
* Major changes to layout and code base, compiles, but needs testing.
* Merge pull request `#2 <https://github.com/segwayrmp/libsegwayrmp/issues/2>`_ from piyushk/master
  remove deadlock in libsegwayrmp.git
* fixed deadlock in ConcurrentQueue when cancelled -- allows libsegwayrmp to terminate gracefully
* Update README.markdown
* Adding `#include <boost/function.hpp>` reported by Manuel Wopfner, Thanks.
* Update include/impl/rmp_serial.h
* Fixing queueing bug.
* Fixing a bug reported by ilan.
* Adding a concurrent queue for processing segwaystatus structures via the user callback, this should allow some flexibility.
* Updated the change log.
* Updating documentation.  I think we are ready for rc2.
* Changed callbacks to use Boost Function typedefs so now you can use Boost Bind to use class methods as callbacks.
  Updated the documentation to reflect the new Boost::bind method of class method callbacks.
  Added a Timestamp callback to allow users to manage time stamping.
  Added an Exception handling callback so users can handle otherwise unhanded exceptions that might occur in the internal library threads.
* Added a conversion bugfix that was reported by David Rajatnam <daver@cse.unsw.edu.au>
* added a changes.txt file to help track changes
* Another README change.
* Updating the README
* Merge branch 'master' of github.com:wjwwood/libsegwayrmp
* Fixed a typo in the CMakeLists.txt file.
* Fixed an issue with angular velocity scaling.
* Updated something to allow doc generation without building.
* Still working on turn commands
* fixed some documentation.
* fixed an angular rate scalar issue.
* Added support for different segwayrmp models, and put in a safety feature for the example.
* Played around with buffer sizes, and I think I got some better performance with no timing losses.
* Fixed an include error that pops up when integrating into ROS.
* Fixed an install error
* Changed the way interface support is dynamically handled.
* More settings for setting limits in the segway internally.
* Added the command to set the max Acceleration limit via the segway interface.
* Somehow managed to break the system while updating documentation... wow
* More documentation updates.
* Fixed a documentation bug.
* Improved the docs.
* Adding a change to make external project integration more friendly.
* More readme changes.
* Forgot to specify the README format.
* Adding stuff to the readme.
* Added some basic documentation.
* Adding a Findsegwayrmp.cmake
* Fixed tests with a special case for testing.
* Added back the struct print out to the example and the errors now print to cerr in the example.
* Added an exception catch in the example.
* Fixed an include problem on Linux.
* Merge branch 'ftdi' of github.com:wjwwood/libsegwayrmp into ftdi
* Reading is working with usb, but writing seems to be happening, but not affecting the segway...
* Fixed some cmake issues.
* Added support for the ftd2xx device interface. Needs more testing.
* Bug fixes for the example program.
* More sofisticated example.
* Everything compiles and passes tests now.
* Started by reorganizing the rmp class to make it truly abstract while leaving as little as possible to the implementation.  Also made the build system a little more sofisticated which should help when using it in other projects.
* fixed a problem with the set max velocity scale factor, but it still isn't working correctly.
* Added the ability to set the max velocity scale factor, which for some reason defaults to 3/4 on startup for the segway, so my connect method sets it to 1:1 by default now.
* Modified the segway example
* Making it more usable by external build systems.
* All parsing tested.
* Still working on the parsing.
* Working on adding some unit tests.
* Driving works, but I still need to look into how the turning is scaled.  Some configurations still cant be set, and I think there are some problems with the sensor data and scales that need to be checked.
* Reading data from the Segway is completely working.  I am having trouble getting my commands that are being sent to work.  I don't know what is going on atm.
* Work in progress, can read packets from the serial port in an abstract way, and the packets are passing validation (header content and checksum).
* Working on issue with checksums, work in progress.
* Build setup complete, stubs make and run.
* Adding initial structure for the serial based segway library.
* first commit
