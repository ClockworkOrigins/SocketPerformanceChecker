
# SocketPerformanceChecker #

Welcome to **SocketPerformanceChecker**!

SocketPerformanceChecker is a tool to measure the performance of different socket implementations provided by libraries such as clockUtils and boost::asio.

## Features ##
 
**General**

 * Easy to use plugin structure to add new socket implementations very easily

## Platforms ##

SocketPerformanceChecker has been tested on:

 * Linux
 * Windows

## Requirements ##

We use CMake (2.8+) for the build system and a C++11 capable compiler such as g++ 4.9 or Visual Studio 13 (or higher). Also Qt 5.x is required for the GUI. Build scripts for all other dependencies are provided in the repository in the dependencies directory.

## Building from source ##

**LINUX**

 * First of all, build all the dependencies.
 * There are several build-scripts, but build-dependencies.sh will run them all
 * You can run the scripts individually, e.g. if you want to rebuild 1 library, but be careful
 * some libraries depend on others (read comments in build-dependencies)

cd dependencies

./build-dependencies.sh

 * Go back to the main directory, run cmake with the appropriate options (see infos below) and finally make

cd ..

cmake .

make

 * now, you'll find the executables in the bin subdirectory and can run it
 * That's all

**WINDOWS**

 * First of all, build all the dependencies. This will take some time.
 * There are several build-scripts, but build-dependencies.bat will run them all
 * You can run the scripts individually, e.g. if you want to rebuild 1 library, but be careful
 * some libraries depends on others (read comments in build-dependencies)
 * build-dependencies.bat and the other build scripts require three tools:
	
 WinRAR.exe (specify path in build-common.bat)
 
 cmake (has to be in path)
 
 Visual Studio
		
 * If all these programs are installed, you start the Visual Studio command line
 * (e.g. Start => All programs => Visual Studio => Visual Studio tools => VS2013 x86 Native Tools)
 * and move to SocketPerformanceChecker directory. Then just follow the next steps:

cd dependencies

build-dependencies.bat

 * Go back to the main directory, run cmake with the appropriate options (see infos below) and you get your Visual Studio project

cd ..

cmake .

 * now you find a SocketPerformanceChecker.sln file you can open in Visual Studio. You can compile it using F7. After this, all library files
 * and the executables are in the bin subdirectory and you can run it
 * That's all
 
##Running the Test##

To run the test the following steps are necessary:

 * Run the SocketPerformanceHelper executable on the PC you want it running
 * Start SocketPerformanceChecker executable either on the same PC or on another one
 * Configure the test as you want to run it. Remember to change the IP in the configuration to the one the SocketPerformanceHelper is running on
 * Click the start button and wait for the results

## Contributing Code ##

We welcome patches. We will set up a Contributor Agreement soon to take care of the legal aspect of contributions.
