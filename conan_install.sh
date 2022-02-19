#!/usr/bin/env sh

rm -rf build 2>/dev/null

cd logger
conan create . "logger/0.1.0@rechip/experimental"
cd ..

conan install . -if ./build/Debug -g cmake_find_package_multi --build missing -s build_type=Debug
conan install . -if ./build/Release -g cmake_find_package_multi --build missing -s build_type=Release
