cmake_minimum_required(VERSION 3.7.2)

project(
  GlosSITarget
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


add_executable( GlosSITarget
  main.cpp
  SteamTarget.cpp
)