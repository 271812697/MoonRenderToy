cmake_minimum_required(VERSION 3.19)
set(TARGET         filaflat)
set(PUBLIC_HDR_DIR include)
# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE ${TARGET}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE ${TARGET}_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${TARGET}_SRC} ${${TARGET}_HEADER})
# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
add_library(${TARGET} STATIC ${${TARGET}_SRC} ${${TARGET}_HEADER})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)








