cmake_minimum_required (VERSION 3.0.0)

##################
# Project settings

project (StimProc VERSION 0.1 LANGUAGES CXX)

######################
# Compilation settings

set(IS_RELEASE true)
option(CERESTIM_STUB # cmake -DCERESTIM_STUB=ON ..
  "Use the CereStim simulator (default ON for non-Windows)" OFF)

if (IS_RELEASE)
  set(CMAKE_BUILD_TYPE Release)
  set(BUILD_TYPE "WIN32")
else (IS_RELEASE)
  set(CMAKE_BUILD_TYPE Debug)
  set(BUILD_TYPE "")
endif (IS_RELEASE)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_CURRENT_SOURCE_DIR}/dist)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/dist)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/dist)

set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-error=unused -Wno-error=unused-parameter")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-variable")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-c-typedef-for-linkage")
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-but-set-variable")
endif()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference -Werror")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRC_HAVE_QT")
if(APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dunix=1")
endif(APPLE)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g -ffast-math -fno-finite-math-only")
set (STIMPROC_DEFINES)

##############
# Source files

if (NOT WIN32)
  set (CERESTIM_STUB ON CACHE BOOL "CereStim only supported on Windows" FORCE)
endif(NOT WIN32)

if (CERESTIM_STUB)
  set (CERESTIM_STUB_FILE src/stub_CereStimDLL.cpp)
  add_definitions (-DCERESTIM_SIMULATOR)
else (CERESTIM_STUB)
  set (CERESTIM_STUB_FILE "")
endif (CERESTIM_STUB)


add_executable (${PROJECT_NAME} "${BUILD_TYPE}"
  src/StimProc.cpp

  src/CereStim.h
  src/CereStim.cpp
  src/CereStimDLL.h
  src/ChannelConf.h
  src/Config.h
  src/Config.cpp
  src/NetClient.h
  src/SPUtils.h
  src/SPUtils.cpp
  src/StimLoop.h
  src/StimLoop.cpp

  ${CERESTIM_STUB_FILE}
)

####################
# External libraries

if (WIN32)
  find_library (CSTIM_LIB "CereStimDLL" PATHS ${PROJECT_SOURCE_DIR}/dist)
  if (NOT CSTIM_LIB)
    message (FATAL_ERROR
      "CereStimDLL library missing from ${PROJECT_SOURCE_DIR}/dist"
    )
  endif ()
else (WIN32)
  set (CSTIM_LIB "")
endif (WIN32)


############################
# Set includes and libraries

target_include_directories (${PROJECT_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries (${PROJECT_NAME}
  ${CSTIM_LIB}
)

target_compile_definitions (${PROJECT_NAME} PUBLIC
  ${STIMPROC_DEFINES}
)


#############################################################
# Update build date-stamp in command-line help on each build.

add_custom_command (TARGET ${PROJECT_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E touch ${PROJECT_SOURCE_DIR}/src/StimProc.cpp
)

