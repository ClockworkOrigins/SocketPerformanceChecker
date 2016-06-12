#!/bin/bash

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

set -e

#
# Automatic build script to compile system dependencies
#

# all paths are relative to this file's directory
cd "$(readlink -f "$(dirname "${0}")")"

. build-common.sh

if [ ! -z "${CLEAN}" ]; then
	title "Cleaning all dependencies..."
else
	title "Compiling all dependencies..."
fi

./build-boost.sh
./build-clockUtils.sh
