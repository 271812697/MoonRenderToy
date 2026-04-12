set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME MoonGeometry)
file(GLOB_RECURSE ${TARGET_NAME}_impl CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.inl")
file(GLOB_RECURSE ${TARGET_NAME}_head CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${${TARGET_NAME}_impl} ${${TARGET_NAME}_head})
add_library(${TARGET_NAME} STATIC ${${TARGET_NAME}_impl} ${${TARGET_NAME}_head})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        ${MayoCore_LinkLibraries}
        ${MayoIO_LinkLibraries}
)
target_link_libraries(${TARGET_NAME} PUBLIC
libboost_thread-vc143-mt-x64-1_85
libboost_chrono-vc143-mt-x64-1_85
libboost_thread-vc143-mt-x64-1_87
libboost_chrono-vc143-mt-x64-1_87
libboost_thread-vc143-mt-gd-x64-1_87
libboost_chrono-vc143-mt-gd-x64-1_87
libboost_thread-vc143-mt-gd-x64-1_85
libboost_chrono-vc143-mt-gd-x64-1_85
)

 target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${Mayo_IncludeDirectories}
)