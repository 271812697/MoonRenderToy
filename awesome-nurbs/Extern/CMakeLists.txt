set(lib_folder "Extern")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(NOT TARGET std_image)
    set(std_image_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/stb_Image)
    file(GLOB_RECURSE std_image_impl CONFIGURE_DEPENDS  
    "${std_image_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE std_image_head CONFIGURE_DEPENDS  
    "${std_image_SOURCE_DIR_}/*.h" "${std_image_SOURCE_DIR_}/*.c")
    source_group(TREE "${std_image_SOURCE_DIR_}" FILES ${std_image_head} ${std_image_impl})
    add_library(std_image STATIC ${std_image_head} ${std_image_impl})
    set_target_properties(std_image PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(std_image PUBLIC $<BUILD_INTERFACE:${std_image_SOURCE_DIR_}/include>)
    set_target_properties(std_image PROPERTIES FOLDER ${lib_folder}/std_image)
endif()

if(NOT TARGET imgui)
    set(imgui_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
    file(GLOB imgui_sources CONFIGURE_DEPENDS  "${imgui_SOURCE_DIR_}/*.cpp")
    file(GLOB imgui_impl CONFIGURE_DEPENDS  
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_glfw.cpp" 
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_glfw.h"
    "${imgui_SOURCE_DIR_}/IconsForkAwesome.h"
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_opengl3.cpp" 
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_opengl3.h"
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_opengl3_loader.h"
    "${imgui_SOURCE_DIR_}/Plugins/ImTextEditor.h",
	"${imgui_SOURCE_DIR_}/Plugins/ImTextEditor.cpp",
	"${imgui_SOURCE_DIR_}/Plugins/imcmd_command_palette.cpp",
	"${imgui_SOURCE_DIR_}/Plugins/imcmd_fuzzy_search.cpp",
	"${imgui_SOURCE_DIR_}/Plugins/implot/implot.h",
	"${imgui_SOURCE_DIR_}/Plugins/implot/implot.cpp",
	"${imgui_SOURCE_DIR_}/Plugins/implot/implot_internal.h",
	"${imgui_SOURCE_DIR_}/Plugins/implot/implot_items.cpp"
)
    set(_imgui_node_editor_Sources
    ${imgui_SOURCE_DIR_}/crude_json.cpp
    ${imgui_SOURCE_DIR_}/crude_json.h
    ${imgui_SOURCE_DIR_}/imgui_bezier_math.h
    ${imgui_SOURCE_DIR_}/imgui_bezier_math.inl
    ${imgui_SOURCE_DIR_}/imgui_canvas.cpp
    ${imgui_SOURCE_DIR_}/imgui_canvas.cpp
    ${imgui_SOURCE_DIR_}/imgui_canvas.h
    ${imgui_SOURCE_DIR_}/imgui_canvas.h
    ${imgui_SOURCE_DIR_}/imgui_extra_math.h
    ${imgui_SOURCE_DIR_}/imgui_extra_math.inl
    ${imgui_SOURCE_DIR_}/imgui_node_editor_api.cpp
    ${imgui_SOURCE_DIR_}/imgui_node_editor_internal.h
    ${imgui_SOURCE_DIR_}/imgui_node_editor_internal.inl
    ${imgui_SOURCE_DIR_}/imgui_node_editor.cpp
    ${imgui_SOURCE_DIR_}/imgui_node_editor.h
    ${imgui_SOURCE_DIR_}/imgui_node_editor.natvis
)
    add_library(imgui STATIC ${imgui_sources} ${imgui_impl} ${_imgui_node_editor_Sources})
    target_link_libraries(imgui PUBLIC glfw)
    target_include_directories(imgui PUBLIC $<BUILD_INTERFACE:${imgui_SOURCE_DIR_}>)
    set_target_properties(imgui PROPERTIES FOLDER ${lib_folder}/imgui)
endif()
if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${lib_folder}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${lib_folder}/glfw)
endif()
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

if(NOT TARGET tinyxml2)
    set(tinyxml2_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
    file(GLOB_RECURSE tinyxml2_impl CONFIGURE_DEPENDS  
    "${tinyxml2_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tinyxml2_head CONFIGURE_DEPENDS  
    "${tinyxml2_SOURCE_DIR_}/*.h")
    add_library(tinyxml2 STATIC ${tinyxml2_impl} ${tinyxml2_head})
    target_include_directories(tinyxml2 PUBLIC $<BUILD_INTERFACE:${tinyxml2_SOURCE_DIR_}>)
    set_target_properties(tinyxml2 PROPERTIES FOLDER ${lib_folder}/tinyxml2)
endif()


if(NOT TARGET tools)   
    set(tools_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    file(GLOB_RECURSE tools_impl CONFIGURE_DEPENDS  
    "${tools_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tools_head CONFIGURE_DEPENDS  
    "${tools_SOURCE_DIR_}/*.h")
    source_group(TREE "${tools_SOURCE_DIR_}" FILES ${tools_impl} ${tools_head})
    add_library(tools STATIC ${tools_impl} ${tools_head})
    target_include_directories(tools PUBLIC $<BUILD_INTERFACE:${tools_SOURCE_DIR_}/include>)
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
if(NOT TARGET ui)
    set(ui_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/UI)
    file(GLOB_RECURSE ui_impl CONFIGURE_DEPENDS  
    "${ui_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE ui_head CONFIGURE_DEPENDS  
    "${ui_SOURCE_DIR_}/*.h")
    source_group(TREE "${ui_SOURCE_DIR_}" FILES ${ui_impl} ${ui_head})
    add_library(ui STATIC ${ui_impl} ${ui_head})
    target_link_libraries(ui PUBLIC imgui)
    target_link_libraries(ui PUBLIC glad)
    target_link_libraries(ui PUBLIC math)
    target_link_libraries(ui PUBLIC tools)
    target_include_directories(ui PUBLIC $<BUILD_INTERFACE:${ui_SOURCE_DIR_}/include>)
    set_target_properties(ui PROPERTIES FOLDER ${lib_folder}/ui)
endif()

if(NOT TARGET window)
    add_definitions(-DUNICODE -D_UNICODE)
    set(window_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Window)
    file(GLOB_RECURSE window_impl CONFIGURE_DEPENDS  
    "${window_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE window_head CONFIGURE_DEPENDS  
    "${window_SOURCE_DIR_}/*.h")
    source_group(TREE "${window_SOURCE_DIR_}" FILES ${window_impl} ${window_head})
    add_library(window STATIC ${window_impl} ${window_head})
    target_link_libraries(window PUBLIC glfw)
    target_link_libraries(window PUBLIC tools)
    target_link_libraries(window PUBLIC std_image)
    target_include_directories(window PUBLIC $<BUILD_INTERFACE:${window_SOURCE_DIR_}>)
    set_target_properties(window PROPERTIES FOLDER ${lib_folder}/window)
endif()

add_subdirectory(libsdl2/tnt)





