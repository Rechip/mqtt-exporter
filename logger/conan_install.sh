#!/bin/env sh

rm -rf build 2>/dev/null

conan remote clean
conan remote add conan-bintray https://conan.bintray.com --force --insert=1
conan remote add conan-center https://center.conan.io --force --insert=2

conan install . -if ./build/Debug -g cmake_find_package_multi --build missing -s build_type=Debug
conan install . -if ./build/Release -g cmake_find_package_multi --build missing -s build_type=Release
