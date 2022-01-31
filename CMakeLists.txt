cmake_minimum_required (VERSION 3.0.0)

##################
# Project settings

project (Elemem VERSION 0.1 LANGUAGES CXX)

######################
# Compilation settings

set(IS_RELEASE true)
option(CEREBUS_HW "Use the Cerebus/Neuroport interface" ON) # cmake -DCEREBUS_HW=OFF ..
option(CERESTIM_STUB # cmake -DCERESTIM_STUB=ON ..
  "Use the CereStim simulator (default ON for non-Windows)" OFF)
option(HDF5_EXPORT "Use HDF5 data export" OFF) # cmake -DHDF5_EXPORT=ON ..

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
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g -ffast-math")
set (ELEMEM_DEFINES)

##############
# Source files

if (NOT WIN32)
  set (CERESTIM_STUB ON CACHE BOOL "CereStim only supported on Windows" FORCE)
endif(NOT WIN32)

if (CERESTIM_STUB)
  set (CERESTIM_STUB_FILE include/stub_CereStimDLL.cpp)
  add_definitions (-DCERESTIM_SIMULATOR)
else (CERESTIM_STUB)
  set (CERESTIM_STUB_FILE "")
endif (CERESTIM_STUB)

if (CEREBUS_HW)
  set (CEREBUS_FILES src/Cerebus.h src/Cerebus.cpp)
  add_definitions (-DCEREBUS_HW)
else (CEREBUS_HW)
  set (CEREBUS_FILES)
endif (CEREBUS_HW)

set (PTSA_INCLUDE_DIRS
  ptsa/ptsa/extensions/morlet
  ptsa/ptsa/extensions/ThreadPool
)
set (PTSA_FILES
  ptsa/ptsa/extensions/morlet/enums.h
  ptsa/ptsa/extensions/morlet/morlet.h
  ptsa/ptsa/extensions/morlet/morlet.cpp
  ptsa/ptsa/extensions/morlet/MorletWaveletTransformMP.h
  ptsa/ptsa/extensions/morlet/MorletWaveletTransformMP.cpp
  ptsa/ptsa/extensions/ThreadPool/ThreadPool.h
)


add_executable (${PROJECT_NAME} "${BUILD_TYPE}"
  src/Elemem.cpp
  #src/api_tests.cpp

  src/About.h
  src/About.cpp
  src/APITests.h
  src/APITests.cpp
  src/ButterworthTransformer.h
  src/ButterworthTransformer.cpp
  src/CereStim.h
  src/CereStim.cpp
  ${CEREBUS_FILES}
  src/CerebusSim.h
  src/CerebusSim.cpp
  ${CERESTIM_STUB_FILE}
  src/ChannelConf.h
  src/ChannelSelector.h
  src/ChannelSelector.cpp
  src/CImage.h
  src/CImage.cpp
  src/Classifier.h
  src/Classifier.cpp
  src/ClassifierEvenOdd.h
  src/ClassifierEvenOdd.cpp
  src/ClassifierLogReg.h
  src/ClassifierLogReg.cpp
  src/ConfigFile.h
  src/ConfigFile.cpp
  src/EDFReplay.h
  src/EDFReplay.cpp
  src/EDFSave.h
  src/EDFSave.cpp
  src/EDFSynch.h
  src/EDFSynch.cpp
  src/EEGAcq.h
  src/EEGAcq.cpp
  src/EEGCircularData.h
  src/EEGCircularData.cpp
  src/EEGData.h
  src/EEGData.cpp
  src/EEGDisplay.h
  src/EEGDisplay.cpp
  src/EEGFileSave.h
  src/EEGFileSave.cpp
  src/EEGPowers.h
  src/EEGPowers.cpp
  src/EEGSource.h
  src/EventLog.h
  src/EventLog.cpp
  src/ExperOPS.h
  src/ExperOPS.cpp
  src/ExpEvent.h
  src/FeatureFilters.h
  src/FeatureFilters.cpp
  src/FeatureWeights.h
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
  src/MainWindow.h
  src/MainWindow.cpp
  src/MorletTransformer.h
  src/MorletTransformer.cpp
  src/NetWorker.h
  src/NetWorker.cpp
  src/NormalizePowers.h
  src/NormalizePowers.cpp
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
  src/RollingStats.h
  src/RollingStats.cpp
  src/Settings.h
  src/Settings.cpp
  src/StatusPanel.h
  src/StatusPanel.cpp
  src/StimGUIConfig.h
  src/StimGUIConfig.cpp
  src/StimWorker.h
  src/StimWorker.cpp
  src/TaskClassifierManager.h
  src/TaskClassifierManager.cpp
  src/TaskClassifierSettings.h
  src/Utils.h
  src/Utils.cpp
  src/WeightManager.h
  src/WeightManager.cpp

  src/Testing.h
  src/Testing.cpp

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
  ${PTSA_FILES}
)

####################
# External libraries

find_package (Qt5 REQUIRED COMPONENTS Core Gui Widgets Concurrent Svg PrintSupport Network)
qt5_use_modules(${PROJECT_NAME} Core Gui Widgets Concurrent Svg PrintSupport Network)

if (HDF5_EXPORT)
  find_package (HDF5 REQUIRED COMPONENTS CXX)
else (HDF5_EXPORT)
  set (HDF5_ENCLUDE_DIRS)
  set (HDF5_LIBRARIES)
  set (ELEMEM_DEFINES ${ELEMEM_DEFINES} NO_HDF5)
endif (HDF5_EXPORT)

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

if (WIN32)
  set (FFTW_LIB "-L${CMAKE_CURRENT_SOURCE_DIR}/lib64 -lfftw3")
else (WIN32)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
  include_directories(PkgConfig::FFTW)
  set(FFTW_LIB PkgConfig::FFTW)
endif (WIN32)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(PTHREAD_LIB Threads::Threads pthread)

# Download submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}")
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ptsa/ptsa/extensions/morlet/morlet.cpp")
  message(FATAL_ERROR "The ptsa submodule was not able to successfully download.  Unable to build.")
endif()


############################
# Set includes and libraries

target_include_directories (${PROJECT_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/src/RC
  ${PROJECT_SOURCE_DIR}/src/RCqt
  ${HDF5_INCLUDE_DIRS}
  ${PTSA_INCLUDE_DIRS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries (${PROJECT_NAME}
  ${CBSDK_LIB}
  ${CSTIM_LIB}
  ${HDF5_LIBRARIES}
  ${FFTW_LIB}
  ${PTHREAD_LIB}
)

target_compile_definitions (${PROJECT_NAME} PUBLIC
  ${ELEMEM_DEFINES}
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

