cmake_minimum_required(VERSION 3.7.2)

project(
  GlosSITarget
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


add_executable(${PROJECT_NAME}
  main.cpp
  SteamTarget.cpp
  TargetWindow.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ../deps/SFML/include)
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  target_link_directories(${PROJECT_NAME} PRIVATE ../deps/SFML/out/Debug/lib)
  target_link_libraries( ${PROJECT_NAME} PRIVATE 
    sfml-system-d
    sfml-window-d
    sfml-graphics-d
    )
else()
  target_link_directories(${PROJECT_NAME} PRIVATE ../deps/SFML/out/Release/lib)
  target_link_libraries( ${PROJECT_NAME} PRIVATE 
    sfml-system
    sfml-window
    sfml-graphics
    )
endif()