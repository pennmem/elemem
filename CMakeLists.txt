cmake_minimum_required(VERSION 3.0.0)

project(elemem VERSION 0.1 LANGUAGES CXX)

set( PROJECT_NAME elemem )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_CURRENT_SOURCE_DIR}/dist )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/dist )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/dist )

add_executable(${PROJECT_NAME}
    "src/api_tests.cpp"
    "src/CereStim.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/RC
)

find_library(CBSDK_LIB "cbsdk" PATHS ${PROJECT_SOURCE_DIR}/lib64)
if(NOT CBSDK_LIB)
    message(FATAL_ERROR "cbsdk library missing from ${PROJECT_SOURCE_DIR}/lib64")
endif()

find_library(CSTIM_LIB "CereStimDLL" PATHS ${PROJECT_SOURCE_DIR}/dll)
if(NOT CSTIM_LIB)
    message(FATAL_ERROR "CereStimDLL library missing from ${PROJECT_SOURCE_DIR}/dll")
endif()

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    ${CBSDK_LIB}
    ${CSTIM_LIB}
)

# Add shared library dependencies to dist/ folder.
if(WIN32)
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
    set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CSTIM_LIB}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif(WIN32)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
