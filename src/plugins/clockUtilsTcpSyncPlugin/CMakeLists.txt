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
set(includedir ${CMAKE_SOURCE_DIR}/include/plugins/clockUtilsTcpSync)

INCLUDE_DIRECTORIES(${includedir})
INCLUDE_DIRECTORIES(${includedir}/../../tool/plugins)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

set(pluginClockUtilsTcpSyncSrc
	${srcdir}/ClockUtilsTcpSyncPlugin.cpp
)

QT5_WRAP_CPP(pluginClockUtilsTcpSyncInclude
	${includedir}/ClockUtilsTcpSyncPlugin.h
)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

add_library(Plugin_ClockUtilsTcpSync SHARED ${pluginClockUtilsTcpSyncSrc} ${pluginClockUtilsTcpSyncInclude})

target_link_libraries(Plugin_ClockUtilsTcpSync ${CLOCKUTILS_LIBRARIES} ${QT_LIBRARIES})

add_custom_command(TARGET Plugin_ClockUtilsTcpSync PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/plugins/sockets)

IF(WIN32)
	add_custom_command(TARGET Plugin_ClockUtilsTcpSync POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/Plugin_ClockUtilsTcpSync.dll ${CMAKE_BINARY_DIR}/bin/plugins/sockets)
ELSEIF(UNIX)
	set_target_properties(Plugin_ClockUtilsTcpSync PROPERTIES PREFIX "")
	add_custom_command(TARGET Plugin_ClockUtilsTcpSync POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink Plugin_ClockUtilsTcpSync.so ${CMAKE_BINARY_DIR}/bin/plugins/sockets/Plugin_ClockUtilsTcpSync.so)
ENDIF(WIN32)
