@echo off

echo -- [ running ] "appveyor build script"

echo -- selecting build target
echo APPVEYOR_REPO_TAG is "%APPVEYOR_REPO_TAG%"

IF "%APPVEYOR_REPO_TAG%"=="true" goto release
REM goto debug

:debug
echo -- building for debug
make
goto :EOF

:release
echo -- building for release
make TARGET=release
goto :EOF
