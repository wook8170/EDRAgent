@echo off

echo  PARAM0 = "%0"
echo  PARAM1 = "%1"
echo  PARAM2 = "%2"
echo  PARAM3 = "%3"
echo  PARAM4 = "%4"

SET TARGET_CONF=%1
echo TARGET_CONF = "%TARGET_CONF%"
rem check DDK path
if not "%1" == "" goto SET_BASE_DDK_PATH
echo specify base DDKPath (param 1)
goto ERROR

:SET_BASE_DDK_PATH
set BASE_DDK_PATH=%1
set BASE_DDK_FILENAME=%~n1

rem Uncomment lines below to set path manually
set BASE_DDK_PATH= c:\WINDDK\7600.16385.1
set BASE_DDK_FILENAME=7600.16385

rem set BASE_DDK_PATH= D:\WINDDK\3790.1830
rem set BASE_DDK_FILENAME=3790.1830

rem set BASE_DDK_PATH= c:\WINDDK\3790
rem set BASE_DDK_FILENAME=3790


if not "%1" == "" goto SET_PROJECT_DIR
echo specify project directory (param 2)
goto ERROR

:SET_PROJECT_DIR
SET PROJECT_DIR=%2

if not "%2" == "" goto SET_DDK_CONF_TYPE
echo specify configuration name
goto ERROR

:SET_DDK_CONF_TYPE
SET PROJECT_CONFIGURATION_NAME=%1

if "%1" == "Debug" SET DDK_CONF_TYPE=chk
if "%1" == "Release" SET DDK_CONF_TYPE=free

if "%DDK_CONF_TYPE%" == "" goto ERROR


set DDK_3790_USED=0
if "%BASE_DDK_FILENAME%" == "3790" (
set DDK_3790_USED=1
)

rem defaults
SET PLATFORM_VERSION=32
SET BUILD_PARAM=-cfIE


:NextArg

if "%3" == "" goto StartBuild
if "%3" == "64" goto Platform64
if "%3" == "-build" goto SetBuildParam

echo Invalid argument: "%3"
goto ERROR


rem ---------------------------------
:Platform64
rem PLATFORM x64
if not "%BASE_DDK_FILENAME%" == "7600.16385" if not "%BASE_DDK_FILENAME%" == "7600" (
	SET PLATFORM_VERSION_TO_PASS= amd64
	goto end64version
)

@echo WDK DETECTED
SET PLATFORM_VERSION_TO_PASS= x64

:end64version

SET PLATFORM_VERSION=64
goto EndCmd;
rem ---------------------------------

rem ---------------------------------
:SetBuildParam

SET BUILD_PARAM=/fIE

goto EndCmd;
rem ---------------------------------

:EndCmd

shift
goto :NextArg

:StartBuild

echo Platform version: %PLATFORM_VERSION%

echo DDK path is %BASE_DDK_PATH%
call %BASE_DDK_PATH%\bin\setenv.bat %BASE_DDK_PATH% %DDK_CONF_TYPE% wnet %PLATFORM_VERSION_TO_PASS%

SET BUILD_ALLOW_LINKER_WARNINGS=1
cd /d %PROJECT_DIR%
build.exe %BUILD_PARAM% %PLATFORM_VERSION_TO_PASS%
goto :EOF
:ERROR
echo FATAL ERROR !