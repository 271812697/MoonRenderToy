set(lib_folder "Extern")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_CXX_STANDARD 20)
if(NOT TARGET glad)
    set(glad_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/glad)
    file(GLOB_RECURSE glad_impl CONFIGURE_DEPENDS  
    "${glad_SOURCE_DIR_}/*.c")
    file(GLOB_RECURSE glad_head CONFIGURE_DEPENDS  
    "${glad_SOURCE_DIR_}/*.h")
    add_library(glad STATIC ${glad_impl} ${glad_head})
    target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${glad_SOURCE_DIR_}/glad > )
    target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${glad_SOURCE_DIR_}>)
   
    set_target_properties(glad PROPERTIES FOLDER ${lib_folder}/glad)
endif()

if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${lib_folder}/spdlog)
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
if(NOT TARGET tinyxml2)   
    set(tinyxml2_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
    file(GLOB_RECURSE tinyxml2_impl CONFIGURE_DEPENDS  
    "${tinyxml2_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tinyxml2_head CONFIGURE_DEPENDS  
    "${tinyxml2_SOURCE_DIR_}/*.h")
    source_group(TREE "${tinyxml2_SOURCE_DIR_}" FILES ${tinyxml2_impl} ${tinyxml2_head})
    add_library(tinyxml2 STATIC ${tinyxml2_impl} ${tinyxml2_head})
    set_target_properties(tinyxml2 PROPERTIES FOLDER ${lib_folder}/tinyxml2)
    target_include_directories(tinyxml2 PUBLIC $<BUILD_INTERFACE:${tinyxml2_SOURCE_DIR_}/include>)
endif()
if(NOT TARGET tracy)   
    set(tracy_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tracy)
    file(GLOB_RECURSE tracy_impl CONFIGURE_DEPENDS  
    "${tracy_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tracy_head CONFIGURE_DEPENDS  
    "${tracy_SOURCE_DIR_}/*.h" "${tracy_SOURCE_DIR_}/*.hpp")
    source_group(TREE "${tracy_SOURCE_DIR_}" FILES ${tracy_impl} ${tracy_head})
    
    
    add_library(tracy STATIC ${tracy_impl} ${tracy_head})
    target_link_libraries(tracy PUBLIC dbghelp.lib  ws2_32.lib)
    target_compile_definitions(tracy PUBLIC 
    TRACY_ENABLE
    TRACY_ON_DEMAND
    TRACY_MEMORY_ENABLE
    )
    target_include_directories(tracy PUBLIC $<BUILD_INTERFACE:${tracy_SOURCE_DIR_}>)
    set_target_properties(tracy PROPERTIES FOLDER ${lib_folder}/tracy)
endif()
