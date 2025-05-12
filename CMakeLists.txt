cmake_minimum_required(VERSION 3.8)
project(UseQt LANGUAGES C CXX)

option(USE_QT6 "Build with Qt6 (Enabled by default)" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT DEFINED EIGEN3_INCLUDE_DIR)
  set(EIGEN3_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Extern/eigen-3.4.0")
endif()
find_package(Eigen3 3.4.0 REQUIRED)

if(USE_QT6)
  find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets)
else()
  find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui OpenGL)
endif()

message(STATUS "QT_VERSION: ${QT_VERSION}, QT_DIR: ${QT_DIR}")

if (${QT_VERSION} VERSION_LESS 5.11.0)
  message(FATAL_ERROR "Requires qt version >= 5.11.0, Your current version is ${QT_VERSION}")
endif()

#SET_PROPERTY(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP auto_gen)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(ThirdPartyDir "${PROJECT_SOURCE_DIR}/Extern")
add_subdirectory(test)
add_subdirectory(Extern)
add_subdirectory(Block)
add_subdirectory(RayTrace)
add_subdirectory(QtSunEarth)
add_subdirectory(DirectX11Init)
add_subdirectory(ScreenWidget)
add_subdirectory(QtNode)
add_subdirectory(Moon)
add_subdirectory(LLGL)


#add_subdirectory(filament)
