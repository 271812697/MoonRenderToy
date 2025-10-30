set(TARGET_NAME Moon)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
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
        Qt${QT_VERSION_MAJOR}::Svg
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
        Dump
        ${MayoCore_LinkLibraries}
        ${MayoIO_LinkLibraries}
)
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${EIGEN3_INCLUDE_DIR}
    ${ThirdPartyDir}
    ${Mayo_IncludeDirectories}
)

target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)

set(OIDN_LIBRARIES "OpenImageDenoise")
target_link_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/oidn/lib
)




# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC ${OIDN_LIBRARIES})



file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/oidn/lib/OpenImageDenoise.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pathtrace/oidn/lib/tbb.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(PATH_TRACE_SCENE_PATH "${CMAKE_SOURCE_DIR}\\Resource\\pathtrace\\scenes\\")
file(TO_CMAKE_PATH "${PATH_TRACE_SCENE_PATH}" PATH_TRACE_SCENE_PATH_CMAKE)
set(PATH_TRACE_HDR_PATH "${CMAKE_SOURCE_DIR}\\Resource\\pathtrace\\scenes\\HDR\\")
file(TO_CMAKE_PATH "${PATH_TRACE_HDR_PATH}" PATH_TRACE_HDR_PATH_CMAKE)
set(PATH_TRACE_SHADER_PATH "${CMAKE_SOURCE_DIR}\\Moon\\pathtrace\\shaders\\")
file(TO_CMAKE_PATH "${PATH_TRACE_SHADER_PATH}" PATH_TRACE_SHADER_PATH_CMAKE)
target_compile_definitions(${TARGET_NAME} PRIVATE PATH_TRACE_SCENE_PATH="${PATH_TRACE_SCENE_PATH_CMAKE}")
target_compile_definitions(${TARGET_NAME} PRIVATE PATH_TRACE_HDR_PATH="${PATH_TRACE_HDR_PATH_CMAKE}")
target_compile_definitions(${TARGET_NAME} PRIVATE PATH_TRACE_SHADER_PATH="${PATH_TRACE_SHADER_PATH_CMAKE}")


set(PROJECT_ENGINE_PATH "${CMAKE_SOURCE_DIR}\\Resource\\Moon\\Data\\Engine\\")
file(TO_CMAKE_PATH "${PROJECT_ENGINE_PATH}" PROJECT_ENGINE_PATH_CMAKE)
target_compile_definitions(${TARGET_NAME} PRIVATE PROJECT_ENGINE_PATH="${PROJECT_ENGINE_PATH_CMAKE}")
set(PROJECT_EDITOR_PATH "${CMAKE_SOURCE_DIR}\\Resource\\Moon\\Data\\Editor\\")
file(TO_CMAKE_PATH "${PROJECT_EDITOR_PATH}" PROJECT_EDITOR_PATH_CMAKE)
target_compile_definitions(${TARGET_NAME} PRIVATE PROJECT_EDITOR_PATH="${PROJECT_EDITOR_PATH_CMAKE}")


set(QtPluginsDir "${QT_DIR}/../../../plugins")
file(GLOB QtPluginIconEnginesDLLs  "${QtPluginsDir}/iconengines/qsvgicon*.dll")
file(GLOB QtPluginImageFormatsDLLs "${QtPluginsDir}/imageformats/qsvg*.dll")
file(GLOB QtPluginPlatformsDLLs    "${QtPluginsDir}/platforms/qwindows*.dll")
set(QtPluginsDLLs ${QtPluginIconEnginesDLLs} ${QtPluginImageFormatsDLLs} ${QtPluginPlatformsDLLs})
foreach(QtPluginDLL ${QtPluginsDLLs})
    cmake_path(GET QtPluginDLL PARENT_PATH QtPluginDLL_Path)
    cmake_path(GET QtPluginDLL_Path FILENAME QtPluginDLL_PathName)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/plugins/${QtPluginDLL_PathName}"
    )
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QtPluginDLL}" "$<TARGET_FILE_DIR:${TARGET_NAME}>/plugins/${QtPluginDLL_PathName}"
    )
endforeach()
# Copy OpenCascade 3rd-party DLLs
foreach(Occ3rdDLL ${OpenCASCADE_3RDPARTY_DLLS})
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${Occ3rdDLL}" $<TARGET_FILE_DIR:${TARGET_NAME}>
    )
endforeach()

# Copy runtime DLLs specified with library IMPORTED_LOCATION property
add_custom_command(
   TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${TARGET_NAME}> $<TARGET_FILE_DIR:${TARGET_NAME}>
    COMMAND_EXPAND_LISTS
)