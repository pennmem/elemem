cmake_minimum_required (VERSION 3.0.0)

##################
# Project settings

project (Elemem VERSION 0.1 LANGUAGES CXX)

######################
# Compilation settings

set(IS_RELEASE true)
option(CEREBUS_STUB "Use the Cerebus simulator" ON) # cmake CEREBUS_STUB .

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

set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-error=unused -Wno-error=unused-parameter")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-variable")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-but-set-variable")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference -Werror")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRC_HAVE_QT")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g -ffast-math")

##############
# Source files

if (WIN32)
  set (OS_SPEC_FILES "")
else (WIN32)
  set (OS_SPEC_FILES include/stub_CereStimDLL.cpp)
endif (WIN32)

if (CEREBUS_STUB)
  set (CEREBUS_FILE include/stub_Cerebus.cpp)
else (CEREBUS_STUB)
  set (CEREBUS_FILE src/Cerebus.cpp)
endif (CEREBUS_STUB)

add_executable (${PROJECT_NAME} "${BUILD_TYPE}"
  src/Elemem.cpp
  #src/api_tests.cpp

  src/About.h
  src/About.cpp
  src/APITests.h
  src/APITests.cpp
  src/CereStim.h
  src/CereStim.cpp
  src/Cerebus.h
  ${CEREBUS_FILE}
  src/ChannelConf.h
  src/ChannelSelector.h
  src/ChannelSelector.cpp
  src/CImage.h
  src/CImage.cpp
  src/ConfigFile.h
  src/ConfigFile.cpp
  src/EDFSave.h
  src/EDFSave.cpp
  src/EEGAcq.h
  src/EEGAcq.cpp
  src/EEGData.h
  src/EEGDisplay.h
  src/EEGDisplay.cpp
  src/EventLog.h
  src/EventLog.cpp
  src/ExperOPS.h
  src/ExperOPS.cpp
  src/ExpEvent.h
  src/GuiParts.h
  src/GuiParts.cpp
  src/Handler.h
  src/Handler.cpp
  src/HDF5Save.h
  src/HDF5Save.cpp
  src/JSONLines.h
  src/JSONLines.cpp
  src/LocGUIConfig.h
  src/LocGUIConfig.cpp
  src/HDF5Save.cpp
  src/MainWindow.h
  src/MainWindow.cpp
  src/NetWorker.h
  src/NetWorker.cpp
  src/OpenConfigDialog.h
  src/OpenConfigDialog.cpp
  src/OPSSpecs.h
  src/Palette.h
  src/Palette.cpp
  src/Popup.h
  src/Popup.cpp
  src/QtFileFunctions.h
  src/QtStyle.h
  src/QtStyle.cpp
  src/RCQApplication.h
  src/RCQApplication.cpp
  src/Settings.h
  src/Settings.cpp
  src/StatusPanel.h
  src/StatusPanel.cpp
  src/StimGUIConfig.h
  src/StimGUIConfig.cpp
  src/StimWorker.h
  src/StimWorker.cpp
  src/Utils.h
  src/Utils.cpp

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
  src/RCqt/Worker.h
  src/RCqt/Worker.cpp

  include/edflib/edflib.cpp

  ${OS_SPEC_FILES}
)

####################
# External libraries

find_package (Qt5 REQUIRED COMPONENTS Core Gui Widgets Concurrent Svg PrintSupport Network)
qt5_use_modules(${PROJECT_NAME} Core Gui Widgets Concurrent Svg PrintSupport Network)

find_package (HDF5 REQUIRED COMPONENTS CXX)

find_library (CBSDK_LIB "cbsdk" PATHS ${PROJECT_SOURCE_DIR}/lib64)
if (NOT CBSDK_LIB)
  message (FATAL_ERROR
    "cbsdk library missing from ${PROJECT_SOURCE_DIR}/lib64"
  )
endif ()

if (WIN32)
  find_library (CSTIM_LIB "CereStimDLL" PATHS ${PROJECT_SOURCE_DIR}/dll)
  if (NOT CSTIM_LIB)
    message (FATAL_ERROR
      "CereStimDLL library missing from ${PROJECT_SOURCE_DIR}/dll"
    )
  endif ()
else (WIN32)
  set (CSTIM_LIB "")
endif (WIN32)

############################
# Set includes and libraries

target_include_directories (${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/src/RC
  ${PROJECT_SOURCE_DIR}/src/RCqt
  ${HDF5_INCLUDE_DIRS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries (${PROJECT_NAME}
  ${CBSDK_LIB}
  ${CSTIM_LIB}
  ${HDF5_LIBRARIES}
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

