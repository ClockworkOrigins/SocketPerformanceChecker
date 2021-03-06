@echo OFF

REM SocketPerformanceChecker
REM Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
REM
REM This file is part of SocketPerformanceChecker; SocketPerformanceChecker is free software; you can redistribute it and/or
REM modify it under the terms of the GNU Lesser General Public
REM License as published by the Free Software Foundation; either
REM version 2.1 of the License, or (at your option) any later version.
REM
REM This library is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
REM Lesser General Public License for more details.
REM
REM You should have received a copy of the GNU Lesser General Public
REM License along with this library; if not, write to the Free Software
REM Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

call build-common.bat %1 %2

Set ARCHIVE=clockutils-1.0.0-src.zip
Set BUILD_DIR=%TMP_DIR%/clockutils-1.0.0-src
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/clockUtils/

IF EXIST %PREFIX% EXIT /B

echo "Compile clockUtils"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% http://clockwork-origins.de/clockUtils/downloads/

cd %BUILD_DIR%

echo "Configuring clockUtils"
cmake -DWITH_TESTING=OFF -DWITH_LIBRARY_COMPRESSION=OFF -DWITH_LIBRARY_CONTAINER=OFF -DWITH_LIBRARY_SOCKETS=ON -DWITH_LIBRARY_INIPARSER=OFF -DWITH_LIBRARY_ARGPARSER=OFF -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building clockUtils"
MSBuild.exe clockUtils.sln /p:Configuration=Release > NUL

echo "Installing clockUtils"
MSBuild.exe INSTALL.vcxproj /p:Configuration=Release > NUL


echo "Cleaning up"
cd %DEP_DIR%
RD /S /Q "%BUILD_DIR%"
RD /S /Q "%TMP_DIR%"
