set(TARGET_NAME Moon)

if (${QT_VERSION_MAJOR} EQUAL 6)
  qt_add_resources(RESOURCES ./resource/resource.qrc)
else()
  qt5_add_resources(RESOURCES ./resource/resource.qrc)
endif()

file(GLOB_RECURSE Moon_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE Moon_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB GLSL ${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/shaders/*.*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${Moon_HEAD} ${Moon_SOURCE} ${GLSL})
add_executable(${TARGET_NAME} ${Moon_HEAD} ${Moon_SOURCE} ${RESOURCES}  ${GLSL})
set_target_properties(${TARGET_NAME}
  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
# Link dependencies    
if(USE_QT6)
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        QtNodes

    )
else()
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGL
        QtNodes
    )
endif()
target_link_libraries(${TARGET_NAME} PUBLIC
        glad
        spdlog
        glm_static
        std_image
        assimp
        tinygltf
        core
)
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${EIGEN3_INCLUDE_DIR}
    ${ThirdPartyDir}
)
target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)

set(OIDN_LIBRARIES "OpenImageDenoise")
target_link_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/oidn/lib
)



# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC ${OIDN_LIBRARIES})



file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/oidn/lib/OpenImageDenoise.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/oidn/lib/tbb.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


set(PROJECT_ENGINE_PATH "${CMAKE_SOURCE_DIR}\\Resource\\Moon\\Data\\Engine\\")
file(TO_CMAKE_PATH "${PROJECT_ENGINE_PATH}" PROJECT_ENGINE_PATH_CMAKE)
target_compile_definitions(${TARGET_NAME} PRIVATE PROJECT_ENGINE_PATH="${PROJECT_ENGINE_PATH_CMAKE}")
set(PROJECT_EDITOR_PATH "${CMAKE_SOURCE_DIR}\\Resource\\Moon\\Data\\Editor\\")
file(TO_CMAKE_PATH "${PROJECT_EDITOR_PATH}" PROJECT_EDITOR_PATH_CMAKE)
target_compile_definitions(${TARGET_NAME} PRIVATE PROJECT_EDITOR_PATH="${PROJECT_EDITOR_PATH_CMAKE}")

