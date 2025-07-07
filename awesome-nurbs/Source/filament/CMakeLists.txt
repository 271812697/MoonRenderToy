cmake_minimum_required(VERSION 3.19)

set(TARGET filaments)

add_subdirectory(filament)
add_subdirectory(libs)
#file(GLOB_RECURSE PUBLIC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/*.c)
#file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_HDRS} ${SRCS})
#add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

#target_link_libraries(${TARGET} utils)
#target_link_libraries(${TARGET} backend)
#target_include_directories(${TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)




