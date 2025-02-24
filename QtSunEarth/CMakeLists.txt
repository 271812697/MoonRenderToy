set(TARGET_NAME SunEarth)
file(GLOB SunEarth_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SunEarth_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/form.ui  ${CMAKE_CURRENT_SOURCE_DIR}/qres.qrc)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SunEarth_HEAD} ${SunEarth_SOURCE})
add_executable(${TARGET_NAME} ${SunEarth_HEAD} ${SunEarth_SOURCE})
# Link dependencies    
if(USE_QT6)
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    )
else()
    target_link_libraries(${TARGET_NAME} PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGL
    )
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



