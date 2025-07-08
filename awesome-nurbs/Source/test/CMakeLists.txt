set(TARGET_NAME TestFilament)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE ${TARGET_NAME}_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE ${TARGET_NAME}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${${TARGET_NAME}_HEAD} ${${TARGET_NAME}_SRC})
add_executable(${TARGET_NAME} ${${TARGET_NAME}_HEAD} ${${TARGET_NAME}_SRC})
target_compile_options(${TARGET_NAME} PRIVATE /bigobj)
# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC glad)
target_link_libraries(${TARGET_NAME} PUBLIC glfw)
target_link_libraries(${TARGET_NAME} PUBLIC window)
target_link_libraries(${TARGET_NAME} PUBLIC filament)
target_link_libraries(${TARGET_NAME} PUBLIC bluegl)
target_link_libraries(${TARGET_NAME} PUBLIC camutils)
target_link_libraries(${TARGET_NAME} PUBLIC sdl2)
target_link_libraries(${TARGET_NAME} PUBLIC sdl2main)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE: ${Extern_DIR}> $<BUILD_INTERFACE: ${CMAKE_CURRENT_SOURCE_DIR}/include>)
set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)



