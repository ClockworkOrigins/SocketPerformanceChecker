# SocketPerformanceChecker
# Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
#
# This file is part of i6engine; i6engine is free software; you can redistribute it and/or
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

set(srcdir .)
set(includedir ${CMAKE_SOURCE_DIR}/include/plugins/clockUtilsTcpASync)

INCLUDE_DIRECTORIES(${includedir})
INCLUDE_DIRECTORIES(${includedir}/../../tool/plugins)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

set(pluginClockUtilsTcpASyncSrc
	${srcdir}/ClockUtilsTcpASyncPlugin.cpp
)

QT5_WRAP_CPP(pluginClockUtilsTcpASyncInclude
	${includedir}/ClockUtilsTcpASyncPlugin.h
)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

add_library(Plugin_ClockUtilsTcpASync SHARED ${pluginClockUtilsTcpASyncSrc} ${pluginClockUtilsTcpASyncInclude})

target_link_libraries(Plugin_ClockUtilsTcpASync ${CLOCKUTILS_LIBRARIES} ${QT_LIBRARIES})

add_custom_command(TARGET Plugin_ClockUtilsTcpASync PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/plugins/sockets)

IF(WIN32)
	add_custom_command(TARGET Plugin_ClockUtilsTcpASync POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/Plugin_ClockUtilsTcpASync.dll ${CMAKE_BINARY_DIR}/bin/plugins/sockets)
ELSEIF(UNIX)
	set_target_properties(Plugin_ClockUtilsTcpASync PROPERTIES PREFIX "")
	add_custom_command(TARGET Plugin_ClockUtilsTcpASync POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/bin/Plugin_ClockUtilsTcpASync.so ${CMAKE_BINARY_DIR}/bin/plugins/sockets/Plugin_ClockUtilsTcpASync.so)
ENDIF(WIN32)
