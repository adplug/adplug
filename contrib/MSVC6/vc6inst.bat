@echo off
rem This batch file installs the distribution files.
rem Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>
rem
rem This file is called from MSVC6! Please do not run it manually!

if "%1" == "" goto manually
if "%2" == "" goto manually

rem Defaults.
set bindir=.
set includedir=.
set libdir=.

rem Read configuration.
if not exist config.bat goto noconfig
call config.bat

rem Check target directory presence.
if not exist "%bindir%" mkdir "%bindir%"
if not exist "%includedir%" mkdir "%includedir%"
if not exist "%libdir%" mkdir "%libdir%"

rem Check for includedir subdirectory.
if not "%3" == "" set includedir=%includedir%\%3
if not exist "%includedir%" mkdir "%includedir%"

rem Install specified file.
if exist %2 goto install
echo Error installing %2 - file not found!
goto end

:install
if "%1" == "b" copy %2 "%bindir%" > nul
if "%1" == "i" copy %2 "%includedir%" > nul
if "%1" == "l" copy %2 "%libdir%" > nul
goto end

:manually
echo This file is called from MSVC6! Please do not run it manually!
goto end

:noconfig
echo No configuration file found!
echo Please refer to the distribution's documentation on how to create one.
goto end

:end
