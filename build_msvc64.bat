@echo off
cd %~dp0
if not exist build md build
cd build
cmake -G "Visual Studio 16 2019" -A Win64 -DCMAKE_BUILD_TYPE=Release ..
if errorlevel 1 cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release ..
