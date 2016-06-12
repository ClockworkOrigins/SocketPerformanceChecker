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

Set ROOT_DIR=%3

call build-common.bat %1 %2

SET SOURCE_DIR=%cd%
SET DEP_DIR=%ROOT_DIR%
Set ARCHIVE=m2etis-0.4.0-src-rev119.zip
Set BUILD_DIR=%cd%\tmp\m2etis-0.4.0-src-rev119
Set PREFIX=%ROOT_DIR%/%ARCH_DIR%/m2etis

call %CONFIG_BAT%

echo "Compile m2etis"

if exist %PREFIX% exit /b

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% http://clockwork-origins.de/m2etis/downloads/

SET DEP_DIR=%ROOT_DIR%

echo "Configuring m2etis"
echo "%DEP_DIR%\..\config\m2etis"

xcopy /I /S /Y "%DEP_DIR%\..\config\m2etis" "%BUILD_DIR%\library\generated\"

cd %BUILD_DIR%

cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -DCMAKE_BUILD_TYPE=Release -DDEP_DIR=%DEP_DIR%/%ARCH_DIR%/ -DWITH_TESTING=OFF -DWITH_SIM=OFF -DWITH_LOGGING=OFF -DWITH_MESSAGECOMPRESSION=OFF -G "%VSCOMPILER%%VSARCH%" .

echo "Building m2etis"
MSBuild.exe m2etis.sln /p:Configuration=Release

echo "Installing m2etis"
MSBuild.exe INSTALL.vcxproj /p:Configuration=Release

echo "Cleaning up"
cd %DEP_DIR%
RD /S /Q "%BUILD_DIR%/.."
