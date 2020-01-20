@echo off
cd %~dp0
if not exist build md build
cd build
cmake -G "Visual Studio 16 2019" -A Win64 -DCMAKE_BUILD_TYPE=Release ..
