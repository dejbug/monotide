@echo off

echo -- [ running ] "appveyor post-build script"

echo -- cleaning up deployment folder
del build\*.o

echo -- ready for deployment
