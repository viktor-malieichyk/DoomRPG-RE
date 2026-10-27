cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

project(DoomRPG LANGUAGES C)
option(DOOMRPG_SHOW_CONSOLE "Show a Windows debug console" OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(WIN32 AND NOT DOOMRPG_SHOW_CONSOLE)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

if(MSVC)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  enable_language(OBJC)
  include(ios/Build.cmake)
  return()
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(FluidSynth REQUIRED)

add_subdirectory("${PROJECT_SOURCE_DIR}/src")