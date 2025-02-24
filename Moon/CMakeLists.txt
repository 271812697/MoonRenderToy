set(TARGET_NAME Moon)

if (${QT_VERSION_MAJOR} EQUAL 6)
  qt_add_resources(RESOURCES ./resource/resource.qrc)
else()
  qt5_add_resources(RESOURCES ./resource/resource.qrc)
endif()

file(GLOB_RECURSE Moon_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE Moon_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${Moon_HEAD} ${Moon_SOURCE})
add_executable(${TARGET_NAME} ${Moon_HEAD} ${Moon_SOURCE} ${RESOURCES})
# Link dependencies    
if(USE_QT6)
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        QtNodes
        glad
    )
else()
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGL
        
        QtNodes
        glad
    )
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})
target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)


