cmake_minimum_required (VERSION 3.0.0)

##################
# Project settings

project (Elemem VERSION 0.1 LANGUAGES CXX)

######################
# Compilation settings

set(IS_RELEASE true)

if (IS_RELEASE)
  set(CMAKE_BUILD_TYPE Release)
  set(BUILD_TYPE "WIN32")
else (IS_RELEASE)
  set(CMAKE_BUILD_TYPE Debug)
  set(BUILD_TYPE "")
endif (IS_RELEASE)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_CURRENT_SOURCE_DIR}/dist)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/dist)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/dist)

set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-error=unused -Wno-error=unused-parameter -Wno-error=unused-but-set-variable -Wno-zero-as-null-pointer-constant -Wnull-dereference -Werror -std=c++17 -O3 -ffast-math")

##############
# Source files

add_executable (${PROJECT_NAME} "${BUILD_TYPE}"
  src/Elemem.cpp
  #src/api_tests.cpp

  src/About.cpp
  src/About.h
  src/CereStim.cpp
  src/CereStim.h
  src/Cerebus.h
  src/Handler.cpp
  src/Handler.h
  src/MainWindow.cpp
  src/MainWindow.h
  src/Popup.cpp
  src/Popup.h
  src/RCQApplication.cpp
  src/RCQApplication.h

  src/RC/APtr.h
  src/RC/Bitfield2D.h
  src/RC/Bitfield3D.h
  src/RC/Bitfield.h
  src/RC/Caller.h
  src/RC/Data1D.h
  src/RC/Data2D.h
  src/RC/Data3D.h
  src/RC/Errors.h
  src/RC/File.h
  src/RC/Iter.h
  src/RC/Macros.h
  src/RC/Net.h
  src/RC/PtrCommon.h
  src/RC/Ptr.h
  src/RC/PtrSharedCommon.h
  src/RC/RCBits.h
  src/RC/RCconfig.h
  src/RC/RC.h
  src/RC/RevPtr.h
  src/RC/RND.h
  src/RC/RStr.h
  src/RC/RTime.h
  src/RC/Tuple.h
  src/RC/Types.h

  src/RCqt/RCqtconfig.h
  src/RCqt/RCqt.h
  src/RCqt/Worker.cpp
  src/RCqt/Worker.h
)

target_include_directories (${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/src/RC
  ${PROJECT_SOURCE_DIR}/src/RCqt
)

####################
# External libraries

find_package (Qt5 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
qt5_use_modules(${PROJECT_NAME} Core Gui Widgets Concurrent)

find_library (CBSDK_LIB "cbsdk" PATHS ${PROJECT_SOURCE_DIR}/lib64)
if (NOT CBSDK_LIB)
  message (FATAL_ERROR
    "cbsdk library missing from ${PROJECT_SOURCE_DIR}/lib64"
  )
endif ()

find_library (CSTIM_LIB "CereStimDLL" PATHS ${PROJECT_SOURCE_DIR}/dll)
if (NOT CSTIM_LIB)
  message (FATAL_ERROR
    "CereStimDLL library missing from ${PROJECT_SOURCE_DIR}/dll"
  )
endif ()

target_link_libraries (${PROJECT_NAME}
  ${CBSDK_LIB}
  ${CSTIM_LIB}
)

########################################################
# Update build date-stamp in About window on each build.

add_custom_command (TARGET ${PROJECT_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E touch ${PROJECT_SOURCE_DIR}/src/About.cpp
)

##################################################
# Add shared library dependencies to dist/ folder.

if (WIN32)
  get_target_property (QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component (QT5_WINDEPLOYQT_EXECUTABLE
    ${QT5_QMAKE_EXECUTABLE} PATH
  )
  set (QT5_WINDEPLOYQT_EXECUTABLE
    "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe"
  )
  if (IS_RELEASE)
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --release $<TARGET_FILE:${PROJECT_NAME}>
    )
  else (IS_RELEASE)
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --debug $<TARGET_FILE:${PROJECT_NAME}>
    )
  endif (IS_RELEASE)

  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CSTIM_LIB}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endif (WIN32)

