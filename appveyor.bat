@ECHO off

REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM
:prepare

ECHO -- [ running ] "stage: prepare"

ECHO -- setting MINGW paths
SET MINGW_BIN=C:\mingw-w64\i686-5.3.0-posix-dwarf-rt_v4-rev0\mingw32\bin
SET PATH=%MINGW_BIN%;%PATH%

ECHO -- renaming mingw32-make.exe in-place
PUSHD %MINGW_BIN%
RENAME mingw32-make.exe make.exe
POPD

ECHO -- [ ready ] for make


REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM
:build

ECHO -- [ running ] "stage: build"

ECHO -- selecting build target

IF "%APPVEYOR_REPO_TAG%"=="true" GOTO build_release
IF "%APPVEYOR_REPO_TAG%"=="false" GOTO build_debug
GOTO :EOF


REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM
:build_debug
ECHO -- building for debug
make

ECHO -- cleaning up build folder
DEL build\*.o

GOTO ok


REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM
:build_release
ECHO -- building for release
make TARGET=release

ECHO -- cleaning up build folder
DEL build\*.o

ECHO -- packing output to deploy
IF NOT EXIST deploy MKDIR deploy
PUSHD build
7z a monotide-%APPVEYOR_REPO_TAG_NAME%.zip *
MOVE monotide-%APPVEYOR_REPO_TAG_NAME%.zip ..\deploy
POPD

ECHO -- deleting build folder
RMDIR /S /Q build

GOTO ok


REM ------ 8< -------- ( snip ) ---------- 8< -------- ( snip ) -------
GOTO :EOF


REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM
:ok
ECHO -- [ OK ] build
GOTO :EOF


REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM REM
:failed
ECHO -- [ FAILED ] build
GOTO :EOF
