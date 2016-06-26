# SocketPerformanceChecker
# Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
#
# This file is part of SocketPerformanceChecker; SocketPerformanceChecker is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# CMake main configfile SocketPerformanceChecker project
#

cmake_minimum_required (VERSION 2.8.12)

#########################################################################
#########################################################################
#
# Global Build Configuration Options
#
#########################################################################

OPTION(WITH_M2ETIS "Build m2etis and the m2etis plugin" OFF)

project(SocketPerformanceChecker)

IF(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

#########################################################################
# Global Macros and Definitions
#########################################################################

# Sets appropriate Compilerflags

set (CXX_FLAGS "-pedantic -Wall -Wextra -Woverloaded-virtual -Wnon-virtual-dtor -Wformat=2 -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wredundant-decls -Wctor-dtor-privacy -Wno-unused-parameter -Wno-long-long -Wno-missing-braces -std=c++11 ")

IF(32BIT)
	set(CXX_FLAGS "${CXX_FLAGS} -m32")
ENDIF(32BIT)

IF(WITH_TESTING)
	set(CXX_FLAGS "${CXX_FLAGS} -fPIC")
ENDIF(WITH_TESTING)
set (CLANG_FLAGS "-Wstring-plus-int")

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	set (CMAKE_CXX_FLAGS_DEBUG		"${CXX_FLAGS} ${CLANG_FLAGS} -g")
	set (CMAKE_CXX_FLAGS_MINSIZEREL		"${CXX_FLAGS} ${CLANG_FLAGS} -0s -DNDEBUG")
	set (CMAKE_CXX_FLAGS_RELEASE		"${CXX_FLAGS} ${CLANG_FLAGS} -O3 -DNDEBUG -Werror")
	set (CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CXX_FLAGS} ${CLANG_FLAGS} -O2 -g -Werror")
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	set (CMAKE_CXX_FLAGS_DEBUG		"${CXX_FLAGS} -g")
	set (CMAKE_CXX_FLAGS_MINSIZEREL		"${CXX_FLAGS} -0s -DNDEBUG")
	set (CMAKE_CXX_FLAGS_RELEASE		"${CXX_FLAGS} -O3 -DNDEBUG -Werror")
	set (CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CXX_FLAGS} -O2 -g -Werror")
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set (CXX_FLAGS "/MP /W4 /wd4127")
	set (CMAKE_CXX_FLAGS_DEBUG		"${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS}")
	set (CMAKE_CXX_FLAGS_MINSIZEREL	"${CMAKE_CXX_FLAGS_MINSIZEREL} ${CXX_FLAGS}")
	set (CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS}")
	set (CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CXX_FLAGS}")
ELSE()
	message (SEND_INFO "Unknown C++ compiler \"${CMAKE_CXX_COMPILER_ID}\". Only GNU, Clang and MSVC are completely supported.")
ENDIF()

# Setting current version
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 9)
SET(VERSION_PATCH 0)

#########################################################################
# Global Directory Definitions
#########################################################################

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/" ${CMAKE_MODULE_PATH})

#Setting Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin)

IF(APPLE)
	set(CMAKE_INSTALL_NAME_DIR @executable_path)
	set(CMAKE_BUILD_ISIXE_INSTALL_RPATH ON)
ENDIF(APPLE)

#########################################################################
# Dependency Path extensions
#########################################################################

IF(WIN32)
	IF(CMAKE_GENERATOR MATCHES "Visual Studio 12")
		SET(DEP_DIR_BUILD "msvc12")
		SET(VS_TOOLCHAIN "msvc12")
		SET(Qt5_Platform msvc2013)
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 14")
		SET(DEP_DIR_BUILD "msvc14")
		SET(VS_TOOLCHAIN "msvc14")
	ELSE(CMAKE_GENERATOR MATCHES "Win64")
		MESSAGE([WARNING] "Toolchain not supported yet!")
	ENDIF()

	IF(CMAKE_GENERATOR MATCHES "Win64")
		SET(DEP_DIR_BUILD "${DEP_DIR_BUILD}_x64")
		SET(64BIT ON)
		SET(VS_ARCH "64")
		SET(Qt5_Platform ${Qt5_Platform}_64)
	ELSEIF(CMAKE_GENERATOR MATCHES "ARM")
		SET(DEP_DIR_BUILD "${DEP_DIR_BUILD}_arm")
	ELSE()
		SET(DEP_DIR_BUILD "${DEP_DIR_BUILD}_x86")
		SET(VS_ARCH "32")
	ENDIF()
	SET(COMPILER_VERSION ${DEP_DIR_BUILD})
ENDIF(WIN32)

SET(ENV{Qt5_DIR} $ENV{Qt5_BaseDir}/${Qt5_Platform})
SET(SPC_DEP_DIR "${CMAKE_SOURCE_DIR}/dependencies/${DEP_DIR_BUILD}" CACHE STRING "path to dependencies for SocketPerformanceChecker")

#########################################################################
# Library Section
#########################################################################

#----------------------------------------------------
# Pthreads
#----------------------------------------------------
IF(UNIX)
	find_package(Threads REQUIRED)
	include_directories(SYSTEM ${PTHREADS_INCLUDE_DIR})
ENDIF(UNIX)

#----------------------------------------------------
# Boost
#---------------------------------------------------
IF(WIN32)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/dependencies/build-boost.bat ${VS_TOOLCHAIN} ${VS_ARCH} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies)
ELSEIF(UNIX)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/dependencies/build-boost.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies)
ENDIF(WIN32)
SET(LIBNAME "BOOST")
SET(LIBHEADER "boost/thread.hpp")
SET(BOOST_ROOT ${SPC_DEP_DIR}/boost)
set(BOOST_COMPONENTS serialization system)
FOREACH(comp ${BOOST_COMPONENTS})
	list(APPEND libs boost_${comp})
ENDFOREACH(comp)
SET(BOOST_COMPONENTS ${libs})
find_package(EasyFind REQUIRED COMPONENTS ${BOOST_COMPONENTS})
include_directories(SYSTEM ${BOOST_INCLUDE_DIR})

#----------------------------------------------------
# clockUtils
#----------------------------------------------------
IF(WIN32)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/dependencies/build-clockUtils.bat ${VS_TOOLCHAIN} ${VS_ARCH} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies)
ELSEIF(UNIX)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/dependencies/build-clockUtils.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies)
ENDIF(WIN32)
SET(LIBNAME "CLOCKUTILS")
SET(LIBHEADER "clockUtils/errors.h")
SET(CLOCKUTILS_ROOT ${SPC_DEP_DIR}/clockUtils)
find_package(EasyFind REQUIRED COMPONENTS clock_sockets)
include_directories(SYSTEM ${CLOCKUTILS_INCLUDE_DIR})

#----------------------------------------------------
# m2etis
#----------------------------------------------------
IF(WITH_M2ETIS)
	# Build m2etis
	IF(WIN32)
		execute_process(COMMAND ${CMAKE_SOURCE_DIR}/dependencies/build-m2etis.bat ${VS_TOOLCHAIN} ${VS_ARCH} "${SPC_DEP_DIR}/.." WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies)
	ELSE(WIN32)
		execute_process(COMMAND ${CMAKE_SOURCE_DIR}/dependencies/build-m2etis.sh "${CMAKE_BUILD_TYPE}" "${SPC_DEP_DIR}" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies)
	ENDIF(WIN32)

	#----------------------------------------------------
	# I6M2ETIS
	#----------------------------------------------------
	SET(LIBNAME "M2etis")
	SET(LIBHEADER "m2etis/pubsub/PubSubSystem.h")
	SET(M2etis_ROOT ${SPC_DEP_DIR}/m2etis)
	find_package(EasyFind REQUIRED COMPONENTS m2etis)
	include_directories(SYSTEM ${M2etis_INCLUDE_DIR})
ENDIF(WITH_M2ETIS)

#----------------------------------------------------
# Qt
#----------------------------------------------------
FIND_PACKAGE(Qt5 REQUIRED Widgets Core Gui Network HINTS $ENV{Qt5_DIR})
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})

SET(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Network_LIBRARIES})

IF(ISIXE_WITH_CONSOLE AND WIN32)
	SET(QT_LIBRARIES ${QT_LIBRARIES} Qt5::WinMain)
ENDIF(ISIXE_WITH_CONSOLE AND WIN32)

####################################################################################
# Install paths
####################################################################################

set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "Library directory name")
## the following are directories where stuff will be installed to
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}" CACHE PATH "The subdirectory to the header prefix")

include_directories(${CMAKE_BINARY_DIR}/include/)

configure_file(${CMAKE_SOURCE_DIR}/config/spcBuildSettings.h.in ${CMAKE_BINARY_DIR}/include/spcBuildSettings.h @ONLY)

ADD_SUBDIRECTORY(src)

IF(WIN32)
	file(GLOB_RECURSE files "${CMAKE_SOURCE_DIR}/dependencies/${DEP_DIR_BUILD}/*.dll")
	foreach(filename ${files})
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${filename} ${CMAKE_BINARY_DIR}/bin)
	endforeach()

	file(GLOB_RECURSE files "$ENV{Qt5_DIR}/bin/icudt*.dll")
	foreach(filename ${files})
		get_filename_component(filename ${filename} NAME)
		configure_file($ENV{Qt5_DIR}/bin/${filename} ${CMAKE_BINARY_DIR}/bin/${filename} COPYONLY)
	endforeach()

	file(GLOB_RECURSE files "$ENV{Qt5_DIR}/bin/icuin*.dll")
	foreach(filename ${files})
		get_filename_component(filename ${filename} NAME)
		configure_file($ENV{Qt5_DIR}/bin/${filename} ${CMAKE_BINARY_DIR}/bin/${filename} COPYONLY)
	endforeach()

	file(GLOB_RECURSE files "$ENV{Qt5_DIR}/bin/icuuc*.dll")
	foreach(filename ${files})
		get_filename_component(filename ${filename} NAME)
		configure_file($ENV{Qt5_DIR}/bin/${filename} ${CMAKE_BINARY_DIR}/bin/${filename} COPYONLY)
	endforeach()

	configure_file($ENV{Qt5_DIR}/bin/libEGL.dll ${CMAKE_BINARY_DIR}/bin/libEGL.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/libGLESv2.dll ${CMAKE_BINARY_DIR}/bin/libGLESv2.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Core.dll ${CMAKE_BINARY_DIR}/bin/Qt5Core.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Gui.dll ${CMAKE_BINARY_DIR}/bin/Qt5Gui.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Network.dll ${CMAKE_BINARY_DIR}/bin/Qt5Network.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Widgets.dll ${CMAKE_BINARY_DIR}/bin/Qt5Widgets.dll COPYONLY)

	configure_file($ENV{Qt5_DIR}/plugins/platforms/qwindows.dll ${CMAKE_BINARY_DIR}/bin/plugins/platforms/qwindows.dll COPYONLY)
ENDIF(WIN32)
