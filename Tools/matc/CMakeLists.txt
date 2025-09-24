cmake_minimum_required(VERSION 3.19)
# ==================================================================================================
# Library
# ==================================================================================================
#set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET matc)
file(GLOB_RECURSE MATC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE MATC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MATC_SRC} ${MATC_HEADER})
# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${MATC_SRC} ${MATC_HEADER})
target_link_libraries(${TARGET} filaflat)
# ==================================================================================================
# Target include
# ==================================================================================================
target_include_directories(${TARGET} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
#target_include_directories(${TARGET} PRIVATE PUBLIC_HDR_DIR)
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)
#set shader path
set(PROJECT_MAT_PATH "${CMAKE_SOURCE_DIR}\\Code\\Renderer\\Shaders\\src\\material\\")
file(TO_CMAKE_PATH "${PROJECT_MAT_PATH}" PROJECT_MAT_PATH_CMAKE)
target_compile_definitions(${TARGET} PRIVATE PROJECT_MAT_PATH="${PROJECT_MAT_PATH_CMAKE}")