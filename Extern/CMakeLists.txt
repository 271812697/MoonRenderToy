set(lib_folder "Extern")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(NOT TARGET glad)
    set(glad_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/glad)
    file(GLOB_RECURSE glad_impl CONFIGURE_DEPENDS  
    "${glad_SOURCE_DIR_}/*.c")
    file(GLOB_RECURSE glad_head CONFIGURE_DEPENDS  
    "${glad_SOURCE_DIR_}/*.h")
    add_library(glad STATIC ${glad_impl} ${glad_head})
    target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${glad_SOURCE_DIR_}>)
    set_target_properties(glad PROPERTIES FOLDER ${lib_folder}/glad)
endif()
if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${lib_folder}/spdlog)
endif()

if(NOT TARGET glm_static)
    add_subdirectory(glm)
    set_target_properties(glm_static PROPERTIES FOLDER ${lib_folder}/glm_static)
endif()


if(NOT TARGET std_image)
    set(std_image_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/stb_Image)
    file(GLOB_RECURSE std_image_impl CONFIGURE_DEPENDS  
    "${std_image_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE std_image_head CONFIGURE_DEPENDS  
    "${std_image_SOURCE_DIR_}/*.h")
    source_group(TREE "${std_image_SOURCE_DIR_}" FILES ${std_image_head} ${std_image_impl})
    add_library(std_image STATIC ${std_image_head} ${std_image_impl})
    set_target_properties(std_image PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(std_image PUBLIC $<BUILD_INTERFACE:${std_image_SOURCE_DIR_}/include>)
    set_target_properties(std_image PROPERTIES FOLDER ${lib_folder}/std_image)
endif()

if(NOT TARGET assimp)
    set(ASSIMP_BUILD_ZLIB ON)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ASSIMP_INSTALL OFF)
    set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
    add_subdirectory(assimp)
    set_target_properties(assimp PROPERTIES FOLDER ${lib_folder}/Assimp)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER ${lib_folder}/Assimp)
    set_target_properties(zlibstatic PROPERTIES FOLDER ${lib_folder}/Assimp)
    #set_target_properties(assimp_cmd PROPERTIES FOLDER ${lib_folder}/Assimp)

endif()

if(NOT TARGET tinygltf)
    set(tinygltf_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
    file(GLOB_RECURSE tinygltf_impl CONFIGURE_DEPENDS  
    "${tinygltf_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tinygltf_head CONFIGURE_DEPENDS  
    "${tinygltf_SOURCE_DIR_}/*.h")
    source_group(TREE "${tinygltf_SOURCE_DIR_}" FILES ${tinygltf_head} ${tinygltf_impl})
    add_library(tinygltf STATIC ${tinygltf_head} ${tinygltf_impl})
    target_link_libraries(tinygltf PUBLIC std_image)
    set_target_properties(tinygltf PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(tinygltf PUBLIC $<BUILD_INTERFACE:${tinygltf_SOURCE_DIR_}/include>)
    set_target_properties(tinygltf PROPERTIES FOLDER ${lib_folder}/tinygltf)
endif()

