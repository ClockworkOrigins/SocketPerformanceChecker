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

set(srcdir .)
set(guidir ${CMAKE_SOURCE_DIR}/gui/tool)
set(includedir ${CMAKE_SOURCE_DIR}/include/tool)

INCLUDE_DIRECTORIES(${includedir})
INCLUDE_DIRECTORIES(${includedir}/../common)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

set(toolSrc
	${srcdir}/main.cpp
	
	${srcdir}/widgets/LineChartWidget.cpp
	${srcdir}/widgets/MainWindow.cpp
	
	${CMAKE_SOURCE_DIR}/src/common/MessageStructs.cpp
)

QT5_WRAP_UI(toolUI
	${guidir}/wndMainWindow.ui
)

QT5_WRAP_CPP(toolGuiHeader
	${includedir}/widgets/LineChartWidget.h
	${includedir}/widgets/MainWindow.h
)

SET(ResourceFile resources/resources.qrc)

SOURCE_GROUP(widgets /widgets/*)
SOURCE_GROUP(moc /moc_*)
SOURCE_GROUP(resources /resources/*)

# add resources ...
QT5_ADD_RESOURCES(QT_RESOURCE_FILES ${ResourceFile})

IF(WIN32)
   set(WINDOWS_RC_FILES
         ${CMAKE_SOURCE_DIR}/resources/common/icon.rc
         )
ENDIF(WIN32)

ADD_EXECUTABLE(SocketPerformanceChecker ${GUI_TYPE} ${toolSrc} ${toolUI} ${toolGuiHeader} ${QT_RESOURCE_FILES} ${WINDOWS_RC_FILES})

target_link_libraries(SocketPerformanceChecker ${BOOST_LIBRARIES} ${CLOCKUTILS_LIBRARIES} ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(SocketPerformanceChecker pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET SocketPerformanceChecker POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/SocketPerformanceChecker.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
