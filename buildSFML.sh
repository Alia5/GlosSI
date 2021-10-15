#!/usr/bin/env bash


cd deps/SFML

cmake -S . -B out/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build out/Debug --config Debug

cmake -S . -B out/Release -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build out/Release --config RelWithDebInfo