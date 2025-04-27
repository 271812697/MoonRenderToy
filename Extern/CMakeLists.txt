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

if(NOT TARGET tools)   
    set(tools_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    file(GLOB_RECURSE tools_impl CONFIGURE_DEPENDS  
    "${tools_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tools_head CONFIGURE_DEPENDS  
    "${tools_SOURCE_DIR_}/*.h")
    source_group(TREE "${tools_SOURCE_DIR_}" FILES ${tools_impl} ${tools_head})
    add_library(tools STATIC ${tools_impl} ${tools_head})
    set_target_properties(tools PROPERTIES FOLDER ${lib_folder}/tools)
endif()

if(NOT TARGET math)
    set(math_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Maths)
    file(GLOB_RECURSE math_impl CONFIGURE_DEPENDS  
    "${math_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE math_head CONFIGURE_DEPENDS  
    "${math_SOURCE_DIR_}/*.h")
    source_group(TREE "${math_SOURCE_DIR_}" FILES ${math_impl} ${math_head})
    add_library(math STATIC ${math_impl} ${math_head})
    target_include_directories(math PUBLIC $<BUILD_INTERFACE:${math_SOURCE_DIR_}/include>)
    set_target_properties(math PROPERTIES FOLDER ${lib_folder}/math)
endif()
if(NOT TARGET lua)
set(lua_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/lua)
add_library(lua STATIC 
${lua_SOURCE_DIR_}/lapi.c
${lua_SOURCE_DIR_}/lauxlib.c
${lua_SOURCE_DIR_}/lbaselib.c
${lua_SOURCE_DIR_}/lcode.c
${lua_SOURCE_DIR_}/lcorolib.c
${lua_SOURCE_DIR_}/lctype.c
${lua_SOURCE_DIR_}/ldblib.c
${lua_SOURCE_DIR_}/ldebug.c
${lua_SOURCE_DIR_}/ldo.c
${lua_SOURCE_DIR_}/ldump.c
${lua_SOURCE_DIR_}/lfunc.c
${lua_SOURCE_DIR_}/lgc.c
${lua_SOURCE_DIR_}/linit.c
${lua_SOURCE_DIR_}/liolib.c
${lua_SOURCE_DIR_}/llex.c
${lua_SOURCE_DIR_}/lmathlib.c
${lua_SOURCE_DIR_}/lmem.c
${lua_SOURCE_DIR_}/loadlib.c
${lua_SOURCE_DIR_}/lobject.c
${lua_SOURCE_DIR_}/lopcodes.c
${lua_SOURCE_DIR_}/loslib.c
${lua_SOURCE_DIR_}/lparser.c
${lua_SOURCE_DIR_}/lstate.c
${lua_SOURCE_DIR_}/lstring.c
${lua_SOURCE_DIR_}/lstrlib.c
${lua_SOURCE_DIR_}/ltable.c
${lua_SOURCE_DIR_}/ltablib.c
#${lua_SOURCE_DIR_}/ltests.c
${lua_SOURCE_DIR_}/ltm.c
#${lua_SOURCE_DIR_}/lua.c
${lua_SOURCE_DIR_}/lundump.c
${lua_SOURCE_DIR_}/lutf8lib.c
${lua_SOURCE_DIR_}/lvm.c
${lua_SOURCE_DIR_}/lzio.c
#${lua_SOURCE_DIR_}/onelua.c
)
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR_})
set_target_properties(lua PROPERTIES FOLDER ${lib_folder}/lua)
endif()

if(NOT TARGET analytics)
    set(analytics_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Analytics)
    file(GLOB_RECURSE analytics_impl CONFIGURE_DEPENDS  
    "${analytics_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE analytics_head CONFIGURE_DEPENDS  
    "${analytics_SOURCE_DIR_}/*.h")
    source_group(TREE "${analytics_SOURCE_DIR_}" FILES ${analytics_impl} ${analytics_head})
    add_library(analytics STATIC ${analytics_impl} ${analytics_head})
    target_include_directories(analytics PUBLIC $<BUILD_INTERFACE:${analytics_SOURCE_DIR_}/include>)
    set_target_properties(analytics PROPERTIES FOLDER ${lib_folder}/analytics)
endif()

if(NOT TARGET Debug)
    set(Debug_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Debug)
    file(GLOB_RECURSE Debug_impl CONFIGURE_DEPENDS  
    "${Debug_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE Debug_head CONFIGURE_DEPENDS  
    "${Debug_SOURCE_DIR_}/*.h" "${Debug_SOURCE_DIR_}/*.inl")
    source_group(TREE "${Debug_SOURCE_DIR_}" FILES ${Debug_impl} ${Debug_head})
    add_library(Debug STATIC ${Debug_impl} ${Debug_head})
    target_link_libraries(Debug PUBLIC tools)
    target_include_directories(Debug PUBLIC $<BUILD_INTERFACE:${Debug_SOURCE_DIR_}/include>)
    set_target_properties(Debug PROPERTIES FOLDER ${lib_folder}/Debug)
endif()

if(NOT TARGET audio)
    set(audio_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Audio)
    file(GLOB_RECURSE audio_impl CONFIGURE_DEPENDS  
    "${audio_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE audio_head CONFIGURE_DEPENDS  
    "${audio_SOURCE_DIR_}/*.h" "${audio_SOURCE_DIR_}/*.inl")
    source_group(TREE "${audio_SOURCE_DIR_}" FILES ${audio_impl} ${audio_head})
    
    add_library(audio STATIC ${audio_impl} ${audio_head})
    target_link_directories(audio PUBLIC	${CMAKE_CURRENT_SOURCE_DIR}/irrklang/lib)
    target_link_libraries(audio PUBLIC tools)
    target_link_libraries(audio PUBLIC Debug)
    target_link_libraries(audio PUBLIC math)
    target_link_libraries(audio PUBLIC irrKlang.lib)
    target_include_directories(audio PUBLIC $<BUILD_INTERFACE:${audio_SOURCE_DIR_}/include>)
    target_include_directories(audio PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/irrklang/include>)
    set_target_properties(audio PROPERTIES FOLDER ${lib_folder}/audio)
endif()

if(NOT TARGET physics)
    set(physics_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Physics)
    file(GLOB_RECURSE physics_impl CONFIGURE_DEPENDS  
    "${physics_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE physics_head CONFIGURE_DEPENDS  
    "${physics_SOURCE_DIR_}/*.h" "${physics_SOURCE_DIR_}/*.inl")
    file(GLOB_RECURSE physics_lib CONFIGURE_DEPENDS  
    "${CMAKE_CURRENT_SOURCE_DIR}/bullet3/lib/Debug/*.lib")
    source_group(TREE "${physics_SOURCE_DIR_}" FILES ${physics_impl} ${physics_head})
    
    add_library(physics STATIC ${physics_impl} ${physics_head})
   
    target_link_libraries(physics PUBLIC tools)
    target_link_libraries(physics PUBLIC Debug)
    target_link_libraries(physics PUBLIC math)
    target_link_libraries(physics PUBLIC ${physics_lib})
    target_include_directories(physics PUBLIC $<BUILD_INTERFACE:${physics_SOURCE_DIR_}/include>)
    target_include_directories(physics PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bullet3/include>)
    set_target_properties(physics PROPERTIES FOLDER ${lib_folder}/physics)
endif()

if(NOT TARGET rendering)
    set(rendering_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Rendering)
    file(GLOB_RECURSE rendering_impl CONFIGURE_DEPENDS  
    "${rendering_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE rendering_head CONFIGURE_DEPENDS  
    "${rendering_SOURCE_DIR_}/*.h" "${rendering_SOURCE_DIR_}/*.inl")
    source_group(TREE "${rendering_SOURCE_DIR_}" FILES ${rendering_impl} ${rendering_head})
    add_library(rendering STATIC ${rendering_impl} ${rendering_head})
    target_link_libraries(rendering PUBLIC tools)
    target_link_libraries(rendering PUBLIC assimp)
    target_link_libraries(rendering PUBLIC glad)
    target_link_libraries(rendering PUBLIC std_image)
    target_link_libraries(rendering PUBLIC Debug)
    target_link_libraries(rendering PUBLIC math)
    target_include_directories(rendering PUBLIC $<BUILD_INTERFACE:${rendering_SOURCE_DIR_}/include>)
    set_target_properties(rendering PROPERTIES FOLDER ${lib_folder}/rendering)
endif()

if(NOT TARGET core)
    set(core_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Core)
    file(GLOB_RECURSE core_impl CONFIGURE_DEPENDS  
    "${core_SOURCE_DIR_}/*.cpp" "${core_SOURCE_DIR_}/*.inl")
    file(GLOB_RECURSE core_head CONFIGURE_DEPENDS  
    "${core_SOURCE_DIR_}/*.h")
    source_group(TREE "${core_SOURCE_DIR_}" FILES ${core_impl} ${core_head})
    add_library(core STATIC ${core_impl} ${core_head})
    
    target_link_libraries(core PUBLIC lua)
    target_link_libraries(core PUBLIC tools)
    target_link_libraries(core PUBLIC std_image)
    target_link_libraries(core PUBLIC physics)
    target_link_libraries(core PUBLIC glad)
    target_link_libraries(core PUBLIC analytics)
    target_link_libraries(core PUBLIC audio)
    target_link_libraries(core PUBLIC Debug)
    target_link_libraries(core PUBLIC math)
    target_link_libraries(core PUBLIC rendering)
    target_link_libraries(core PUBLIC tools)

    target_include_directories(core PUBLIC $<BUILD_INTERFACE:${core_SOURCE_DIR_}/include>)
    set_target_properties(core PROPERTIES FOLDER ${lib_folder}/core)
    target_compile_options(core PRIVATE /bigobj )
    #target_compile_options(core PRIVATE /bigobj )
endif()
