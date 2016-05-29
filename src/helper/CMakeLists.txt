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
set(includedir ${CMAKE_SOURCE_DIR}/include/helper)

INCLUDE_DIRECTORIES(${includedir})
INCLUDE_DIRECTORIES(${includedir}/../common)
INCLUDE_DIRECTORIES(${includedir}/../tool/plugins)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

set(helperSrc
	${srcdir}/main.cpp

	${srcdir}/Helper.cpp

	${CMAKE_SOURCE_DIR}/src/common/MessageStructs.cpp
)

ADD_EXECUTABLE(SocketPerformanceHelper ${helperSrc})

target_link_libraries(SocketPerformanceHelper ${BOOST_LIBRARIES} ${CLOCKUTILS_LIBRARIES} ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(SocketPerformanceHelper pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET SocketPerformanceHelper POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/SocketPerformanceHelper.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)