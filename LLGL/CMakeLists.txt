cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# === Target platforms ===

if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    set(LLGL_BUILD_64BIT ON)
else()
    set(LLGL_BUILD_64BIT OFF)
endif()

if(LLGL_BUILD_64BIT)
    set(LLGL_TARGET_PLATFORM "Win64")
else()
    set(LLGL_TARGET_PLATFORM "Win32")
endif()

#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)

set( EXTERNAL_MODULE_DIR            "${CMAKE_CURRENT_SOURCE_DIR}/cmake"                   )
set( PROJECT_INCLUDE_DIR            "${CMAKE_CURRENT_SOURCE_DIR}/include"                 )
set( EXTERNAL_INCLUDE_DIR           "${CMAKE_CURRENT_SOURCE_DIR}/external"                )
set( EXAMPLE_PROJECTS_ROOT_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/examples"                )
set( EXAMPLE_CPP_PROJECTS_DIR       "${EXAMPLE_PROJECTS_ROOT_DIR}/Cpp"              )
set( SHARED_PLATFORM_DIR            "${EXAMPLE_PROJECTS_ROOT_DIR}/Shared/Platform"  )
set( SHARED_ASSETS_DIR              "${EXAMPLE_PROJECTS_ROOT_DIR}/Shared/Assets"    )
set( BACKEND_INCLUDE_DIR            "${PROJECT_INCLUDE_DIR}/LLGL/Backend"           )

# === Macros ===
set(LLGL_MOBILE_PLATFORM OFF)

macro(ADD_DEFINE IDENT)
    add_definitions("-D${IDENT}")
endmacro()

macro(ADD_PROJECT_DEFINE TARGET_NAME IDENT)
    target_compile_definitions(${TARGET_NAME} PRIVATE "-D${IDENT}")
endmacro()

macro(ADD_DEBUG_DEFINE IDENT)
    # Requires CMake 3.12
    add_compile_definitions("$<$<CONFIG:Debug>:${IDENT}>")
endmacro()

# === Functions ===

# Filters sources files with a variadic argument for all directores:
# Example:
#   find_source_files(OutputFiles "*.h;*.cpp" ${MyProjectDir}/Base ${MyProjectDir}/Extensions)
function(find_source_files OUTPUT_LIST FILTERS)
    # Substitute predefined filters
    if("${FILTERS}" STREQUAL "C")
        set(FILTERS "*.c;*.h;*.inl")
    elseif("${FILTERS}" STREQUAL "CXX")
        set(FILTERS "*.c;*.cpp;*.h;*.inl")
    elseif("${FILTERS}" STREQUAL "OBJC")
        set(FILTERS "*.c;*.cpp;*.h;*.inl;*.m;*.mm")
    elseif("${FILTERS}" STREQUAL "RES")
        set(FILTERS "*.c;*.cpp;*.h;*.inl;*.m;*.mm;*.metal;*.vert;*.frag;*.tesc;*.tese")
    elseif("${FILTERS}" STREQUAL "INC")
        set(FILTERS "*.h;*.hpp;*.inl")
    endif()

    # Collect all input directories for each filter
    set(FilteredProjectDirs "")
    set(ProjectDirs "${ARGN}")
    foreach(ProjectSubdir ${ProjectDirs})
        foreach(Filter ${FILTERS})
            list(APPEND FilteredProjectDirs "${ProjectSubdir}/${Filter}")
        endforeach()
    endforeach()
    
    # Find all source files
    file(GLOB FilteredProjectFiles ${FilteredProjectDirs})
    
    # Write filtered files to output variable
    set(${OUTPUT_LIST} ${${OUTPUT_LIST}} ${FilteredProjectFiles} PARENT_SCOPE)
endfunction()

# Adds a (variadic) list of resource files to the output project files.
# The resource files must be located in either the shared assets folder,
# i.e. either under "examples/Shared/Assets/".
# Example:
#   add_project_resource_files(MyProjectFiles "Models/ModelAssets-*.obj" "Textures/TextureAsset-1.jpg")
function(add_project_resource_files OUTPUT_LIST SRC_FOLDER)
    # Iterate over all input filenames and expand their paths depending on their file extensions
    set(ResourceFiles "")
    set(InputFilenames "${ARGN}")
    foreach(InputFile ${InputFilenames})
        if("${InputFile}" MATCHES ".+\\.(png|jpg|dds|tga|obj|txt|map|icns)")
            file(GLOB SharedResourceFiles "${SHARED_ASSETS_DIR}/${InputFile}")
            file(GLOB ProjectResourceFiles "${SRC_FOLDER}/${InputFile}")
            list(APPEND ResourceFiles "${SharedResourceFiles}" "${ProjectResourceFiles}")
        endif()
    endforeach()
    
    # Write expanded resource filenames to output variable
    set(${OUTPUT_LIST} ${${OUTPUT_LIST}} ${ResourceFiles} PARENT_SCOPE)
endfunction()

# Bundles input source files together with platform specific resource files, e.g. macOS and iOS bundles.
function(bundle_project_files OUTPUT_LIST SRC_FILES)
    set(BundledProjectFiles "${SRC_FILES}")
    set(${OUTPUT_LIST} ${BundledProjectFiles} PARENT_SCOPE)
endfunction()

# Main function to find and bundle project source files.
# @param SRC_FILES: can either be a directory or a list of specific source files if it sends with ".cpp", ".c", ".h", ".m", or ".mm".
function(find_project_source_files OUTPUT_LIST SRC_FILES)
    if("${SRC_FILES}" MATCHES ".+\\.(cpp|c|h|m|mm|cs)")
        set(ProjectFiles "${SRC_FILES}")
    else()
        # Find all source files for project
        find_source_files(ProjectFiles CXX ${SRC_FILES})
    endif()
    
    # Bundle source files and write to output variable
    bundle_project_files(OutputProjectFiles "${ProjectFiles}")

    # Write final project file list to output parameter
    set(${OUTPUT_LIST} ${OutputProjectFiles} PARENT_SCOPE)
endfunction()

function(set_llgl_module_properties MODULE_NAME)
    set_target_properties(
        ${MODULE_NAME} PROPERTIES
            LINKER_LANGUAGE CXX
            DEBUG_POSTFIX   "D"
            FOLDER          "LLGL"
    )
endfunction()

function(append_global_property PROP_NAME OUTPUT_LIST NEW_ITEM)
    get_property(PropertyList GLOBAL PROPERTY ${PROP_NAME})

    # Link base libs first to avoid wrong order in GCC.
    # For Clang, this order can cause duplicate linking entries, but shouldn't hurt.
    # For now it's not worth the trouble having to distinguish between compiler toolchains (see CMAKE_CXX_COMPILER_ID).
    list(APPEND PropertyList ${NEW_ITEM})

    set_property(GLOBAL PROPERTY ${PROP_NAME} "${PropertyList}")
    set(${OUTPUT_LIST} "${PropertyList}" PARENT_SCOPE)
endfunction()

function(add_llgl_module MODULE_NAME DEFINE_NAME SRC_FILES)
    # Add module as either static or shared library
    if(LLGL_BUILD_STATIC_LIB)
        add_library(${MODULE_NAME} STATIC ${SRC_FILES})
    else()
        add_library(${MODULE_NAME} SHARED ${SRC_FILES})
    endif()
    
    # Keep track of all active module names
    append_global_property(LLGL_GLOBAL_MODULE_LIST None ${MODULE_NAME})
    
    # Add basic link libraries and properties
    set_llgl_module_properties(${MODULE_NAME})
    
    ADD_PROJECT_DEFINE(LLGL ${DEFINE_NAME})
endfunction()

# Sets the working directory for startup for the specified project.
function(set_project_working_dir PROJECT_NAME WORKING_DIR)
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.8.2")
        set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIR}"
        )
    endif()
endfunction()

function(add_llgl_example_project PROJECT_NAME LINKER_LANG SRC_FILES LIB_FILES)
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    target_link_libraries(${PROJECT_NAME} ${LIB_FILES})
    # Configure linker settings
    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE ${LINKER_LANG} DEBUG_POSTFIX "D")

    string(SUBSTRING ${PROJECT_NAME} 8 -1 PROJECT_BASE_NAME)
    set_project_working_dir(${PROJECT_NAME} "${EXAMPLE_CPP_PROJECTS_DIR}/${PROJECT_BASE_NAME}")
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")

endfunction()

# === Preprocessor definitions ===

if(WIN32)
    ADD_DEFINE(_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINE(_SCL_SECURE_NO_WARNINGS)
    ADD_DEFINE(UNICODE)
    ADD_DEFINE(NOMINMAX)
    ADD_DEFINE(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
    if(MSVC)
        # Disable some warnings for MSVC compiler
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4251 /wd4290 /wd4103")
    endif(MSVC)
endif(WIN32)

ADD_DEBUG_DEFINE(LLGL_DEBUG)

# === Initialize summary variables ===

set(SUMMARY_LIBRARY_TYPE "Unknown")
set(SUMMARY_TARGET_ARCH "Unknown")
set(SUMMARY_FLAGS "")

# === Options ===

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(LLGL_BUILD_64BIT 1)
else()
    set(LLGL_BUILD_64BIT 0)
endif()

option(LLGL_ENABLE_CHECKED_CAST "Enable dynamic checked cast (only in Debug mode)" ON)
option(LLGL_ENABLE_DEBUG_LAYER "Enable renderer debug layer (for both Debug and Release mode)" ON)
option(LLGL_ENABLE_EXCEPTIONS "Enable C++ exceptions" OFF)

option(LLGL_PREFER_STL_CONTAINERS "Prefers C++ STL containers over custom containers, e.g. std::vector over SmallVector<T>" OFF)

option(LLGL_BUILD_STATIC_LIB "Build LLGL as static library" OFF)


option(LLGL_BUILD_RENDERER_OPENGL "Include OpenGL renderer project" ON)

if(WIN32)
    option(LLGL_BUILD_RENDERER_DIRECT3D11 "Include Direct3D11 renderer project" ON)
    option(LLGL_BUILD_RENDERER_DIRECT3D12 "Include Direct3D12 renderer project" ON)
endif()

option(LLGL_BUILD_WRAPPER_C99 "Include wrapper for C99" OFF)
option(LLGL_BUILD_WRAPPER_CSHARP "Include wrapper for C# (requires LLGL_BUILD_WRAPPER_C99)" OFF)

if(NOT LLGL_BUILD_WRAPPER_C99)
    if(LLGL_BUILD_WRAPPER_CSHARP)
        message(SEND_ERROR "LLGL_BUILD_WRAPPER_CSHARP is enabled but not LLGL_BUILD_WRAPPER_C99; The C# wrapper depends on the C99 wrapper!")
    endif()
endif()

if(LLGL_ENABLE_CHECKED_CAST)
    ADD_DEBUG_DEFINE(LLGL_ENABLE_CHECKED_CAST)
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    ADD_DEFINE(LLGL_ENABLE_DEBUG_LAYER)
endif()

if(LLGL_ENABLE_EXCEPTIONS)
    ADD_DEFINE(LLGL_ENABLE_EXCEPTIONS)
endif()

if(LLGL_BUILD_STATIC_LIB)
    ADD_DEFINE(LLGL_BUILD_STATIC_LIB)
endif()

if(LLGL_PREFER_STL_CONTAINERS)
    ADD_DEFINE(LLGL_PREFER_STL_CONTAINERS)
endif()

if(LLGL_MACOS_ENABLE_COREVIDEO)
    ADD_DEFINE(LLGL_MACOS_ENABLE_COREVIDEO)
endif()

if(LLGL_WASM_PLATFORM)
    set(SUMMARY_TARGET_ARCH "wasm")
elseif(LLGL_MOBILE_PLATFORM)
    if("${ANDROID_ABI}" STREQUAL "x86_64")
        set(ARCH_AMD64 ON)
        set(SUMMARY_TARGET_ARCH "x86-64")
    elseif("${ANDROID_ABI}" STREQUAL "x86")
        set(ARCH_IA32 ON)
        set(SUMMARY_TARGET_ARCH "x86")
    else()
        set(ARCH_ARM64 ON)
        set(SUMMARY_TARGET_ARCH "arm64")
    endif()
elseif(APPLE OR LLGL_BUILD_64BIT)
    set(ARCH_AMD64 ON)
    set(SUMMARY_TARGET_ARCH "x86-64")
else()
    set(ARCH_IA32 ON)
    set(SUMMARY_TARGET_ARCH "x86")
endif()



# === Global files ===

set(FilesMsvcNatvis ${CMAKE_CURRENT_SOURCE_DIR}/LLGL.natvis)

# Common files
find_source_files(FilesInclude                      "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL")
find_source_files(FilesIncludeBackend               "*.inl" "${PROJECT_INCLUDE_DIR}/LLGL/Backend")
find_source_files(FilesIncludeContainer             "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL/Container")
find_source_files(FilesIncludeUtils                 "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL/Utils")
find_source_files(FilesIncludePlatformBase          "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL/Platform")
find_source_files(FilesCore                         CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Core")
find_source_files(FilesPlatformBase                 CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Platform")
find_source_files(FilesRenderer                     CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Renderer")

if(LLGL_ENABLE_DEBUG_LAYER)
    find_source_files(FilesRendererDbg              CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Renderer/DebugLayer")
    find_source_files(FilesRendererDbgBuffer        CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Renderer/DebugLayer/Buffer")
    find_source_files(FilesRendererDbgRenderState   CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Renderer/DebugLayer/RenderState")
    find_source_files(FilesRendererDbgShader        CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Renderer/DebugLayer/Shader")
    find_source_files(FilesRendererDbgTexture       CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Renderer/DebugLayer/Texture")
endif()


find_source_files(FilesPlatform             CXX     "${CMAKE_CURRENT_SOURCE_DIR}/sources/Platform/Win32")
find_source_files(FilesIncludePlatform      CXX     "${PROJECT_INCLUDE_DIR}/LLGL/Platform/Win32")


# === Source group folders ===

source_group("NatVis" FILES ${FilesMsvcNatvis})

source_group("Include"              FILES ${FilesInclude})
source_group("Include\\Backend"     FILES ${FilesIncludeBackend})
source_group("Include\\Container"   FILES ${FilesIncludeContainer})
source_group("Include\\Utils"       FILES ${FilesIncludeUtils})
source_group("Include\\Platform"    FILES ${FilesIncludePlatformBase} ${FilesIncludePlatform})

source_group("Sources\\Core"        FILES ${FilesCore})
source_group("Sources\\Platform"    FILES ${FilesPlatformBase} ${FilesPlatform})
source_group("Sources\\Renderer"    FILES ${FilesRenderer})

if(LLGL_ENABLE_DEBUG_LAYER)
    source_group("Sources\\Renderer\\DebugLayer"                FILES ${FilesRendererDbg})
    source_group("Sources\\Renderer\\DebugLayer\\Buffer"        FILES ${FilesRendererDbgBuffer})
    source_group("Sources\\Renderer\\DebugLayer\\RenderState"   FILES ${FilesRendererDbgRenderState})
    source_group("Sources\\Renderer\\DebugLayer\\Shader"        FILES ${FilesRendererDbgShader})
    source_group("Sources\\Renderer\\DebugLayer\\Texture"       FILES ${FilesRendererDbgTexture})
endif()

# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")


# === Projects ===

set(
    FilesLLGL
    ${FilesInclude}
    ${FilesIncludeBackend}
    ${FilesIncludeContainer}
    ${FilesIncludeUtils}
    ${FilesIncludePlatformBase}
    ${FilesIncludePlatform}
    ${FilesCore}
    ${FilesPlatformBase}
    ${FilesPlatform}
    ${FilesRenderer}
)



if(LLGL_ENABLE_DEBUG_LAYER)
    list(
        APPEND FilesLLGL
        ${FilesRendererDbg}
        ${FilesRendererDbgBuffer}
        ${FilesRendererDbgRenderState}
        ${FilesRendererDbgShader}
        ${FilesRendererDbgTexture}
    )
endif()



# Base project
if(LLGL_BUILD_STATIC_LIB)
    set(SUMMARY_LIBRARY_TYPE "Static")
    add_library(LLGL STATIC ${FilesLLGL})
else()
    set(SUMMARY_LIBRARY_TYPE "Shared")
    add_library(LLGL SHARED ${FilesLLGL})
endif()

include(GNUInstallDirs)
target_include_directories(LLGL INTERFACE
                           $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


set_property(GLOBAL PROPERTY LLGL_GLOBAL_MODULE_LIST LLGL)

set_llgl_module_properties(LLGL)


if(LLGL_BUILD_RENDERER_OPENGL OR LLGL_BUILD_RENDERER_OPENGLES3 OR LLGL_BUILD_RENDERER_WEBGL)
    add_subdirectory(sources/Renderer/OpenGL)
endif()

# Static libs must all be linked to the final apps (LLGL_BUILD_STATIC_LIB).
# Also UWP apps need references to all loaded modules (LLGL_UWP_PLATFORM).
if(LLGL_BUILD_STATIC_LIB OR LLGL_UWP_PLATFORM)
    get_property(LLGL_MODULE_LIBS GLOBAL PROPERTY LLGL_GLOBAL_MODULE_LIST)
else()
    set(LLGL_MODULE_LIBS LLGL)
endif()

set(GaussLib_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/GaussianLib")
add_subdirectory(examples)

# Install targets, headers, and CMake config files
get_property(LLGL_ALL_TARGETS GLOBAL PROPERTY LLGL_GLOBAL_MODULE_LIST)


install(TARGETS ${LLGL_ALL_TARGETS} EXPORT LLGLTargets RUNTIME LIBRARY ARCHIVE)
install(DIRECTORY "${PROJECT_INCLUDE_DIR}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT LLGLTargets NAMESPACE LLGL:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/LLGL")
include(CMakePackageConfigHelpers)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LLGLConfig.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/LLGL")

# Summary Information
message(STATUS "~~~ Build Summary ~~~")

message(STATUS "Target Platform: ${LLGL_TARGET_PLATFORM}")
message(STATUS "Target Architecture: ${SUMMARY_TARGET_ARCH}")
message(STATUS "Target Library: ${SUMMARY_LIBRARY_TYPE}")


if(LLGL_BUILD_RENDERER_OPENGL)
    message(STATUS "Build Renderer: OpenGL")
endif()


if(LLGL_BUILD_EXAMPLES)
    message(STATUS "Build Examples")
endif()


if(LLGL_VK_ENABLE_SPIRV_REFLECT)
    message(STATUS "Including Submodule: SPIRV-Headers")
endif()

if(NOT "${SUMMARY_FLAGS}" STREQUAL "")
    message(STATUS "Options: ${SUMMARY_FLAGS}")
endif()

message(STATUS "~~~~~~~~~~~~~~~~~~~~~")

