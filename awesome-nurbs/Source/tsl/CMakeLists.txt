cmake_minimum_required(VERSION 3.19)
set(TARGET tsl)

file(GLOB_RECURSE ${TARGET}_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${TARGET} STATIC ${${TARGET}_HEAD})
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)