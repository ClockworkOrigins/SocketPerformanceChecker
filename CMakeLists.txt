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

option(WITH_BOOST_ASIO "build SocketPerformanceChecker with tests" ON)

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
SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 0)
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
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 14")
		SET(DEP_DIR_BUILD "msvc14")
	ELSE(CMAKE_GENERATOR MATCHES "Win64")
		MESSAGE([WARNING] "Toolchain not supported yet!")
	ENDIF()

	IF(CMAKE_GENERATOR MATCHES "Win64")
		SET(DEP_DIR_BUILD "${DEP_DIR_BUILD}_x64")
		SET(64BIT ON)
	ELSEIF(CMAKE_GENERATOR MATCHES "ARM")
		SET(DEP_DIR_BUILD "${DEP_DIR_BUILD}_arm")
	ELSE()
		SET(DEP_DIR_BUILD "${DEP_DIR_BUILD}_x86")
	ENDIF()
	SET(COMPILER_VERSION ${DEP_DIR_BUILD})
ENDIF(WIN32)

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
SET(LIBNAME "BOOST")
SET(LIBHEADER "boost/thread.hpp")
SET(BOOST_ROOT ${SPC_DEP_DIR}/boost)
set(BOOST_COMPONENTS serialization)
FOREACH(comp ${BOOST_COMPONENTS})
	list(APPEND libs boost_${comp})
ENDFOREACH(comp)
SET(BOOST_COMPONENTS ${libs})
find_package(EasyFind REQUIRED COMPONENTS ${BOOST_COMPONENTS})
include_directories(SYSTEM ${BOOST_INCLUDE_DIR})

#----------------------------------------------------
# clockUtils
#----------------------------------------------------
SET(LIBNAME "CLOCKUTILS")
SET(LIBHEADER "clockUtils/errors.h")
SET(CLOCKUTILS_ROOT ${SPC_DEP_DIR}/clockUtils)
find_package(EasyFind REQUIRED COMPONENTS clock_sockets)
include_directories(SYSTEM ${CLOCKUTILS_INCLUDE_DIR})

#----------------------------------------------------
# Qt
#----------------------------------------------------
IF(64BIT)
	SET(ENV{Qt5_DIR} "$ENV{Qt5_DIR}_64")
ENDIF(64BIT)
FIND_PACKAGE(Qt5 REQUIRED Widgets Core Gui)
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})

SET(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES})

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

ADD_SUBDIRECTORY(src)

IF(WIN32)
	file(GLOB_RECURSE files "${CMAKE_SOURCE_DIR}/dependencies/${DEP_DIR_BUILD}/*.dll")
	foreach(filename ${files})
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${filename} ${CMAKE_BINARY_DIR}/bin)
	endforeach()
ENDIF(WIN32)

# set symlinks if it is an out-of-source-build
IF(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	IF(UNIX)
		execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/dependencies/${DEP_DIR_BUILD} ${CMAKE_BINARY_DIR}/dependencies/${DEP_DIR_BUILD})
	ENDIF()
ENDIF(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
