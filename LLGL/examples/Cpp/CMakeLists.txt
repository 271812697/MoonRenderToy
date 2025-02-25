if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()
# === Source files ===

# Example project files
find_source_files(FilesExampleBase CXX "${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase")



set(FilesExampleBaseAll ${FilesExampleBase})


find_project_source_files( FilesExample_Animation           "${EXAMPLE_CPP_PROJECTS_DIR}/Animation"        )
find_project_source_files( FilesExample_ClothPhysics        "${EXAMPLE_CPP_PROJECTS_DIR}/ClothPhysics"     )
find_project_source_files( FilesExample_Fonts               "${EXAMPLE_CPP_PROJECTS_DIR}/Fonts"            )
find_project_source_files( FilesExample_HelloGame           "${EXAMPLE_CPP_PROJECTS_DIR}/HelloGame"        )
find_project_source_files( FilesExample_HelloTriangle       "${EXAMPLE_CPP_PROJECTS_DIR}/HelloTriangle"    )
find_project_source_files( FilesExample_Instancing          "${EXAMPLE_CPP_PROJECTS_DIR}/Instancing"       )
find_project_source_files( FilesExample_IndirectDraw        "${EXAMPLE_CPP_PROJECTS_DIR}/IndirectDraw"     )
find_project_source_files( FilesExample_MultiContext        "${EXAMPLE_CPP_PROJECTS_DIR}/MultiContext"     )
find_project_source_files( FilesExample_MultiRenderer       "${EXAMPLE_CPP_PROJECTS_DIR}/MultiRenderer"    )
find_project_source_files( FilesExample_MultiThreading      "${EXAMPLE_CPP_PROJECTS_DIR}/MultiThreading"   )
find_project_source_files( FilesExample_PBR                 "${EXAMPLE_CPP_PROJECTS_DIR}/PBR"              )
find_project_source_files( FilesExample_PostProcessing      "${EXAMPLE_CPP_PROJECTS_DIR}/PostProcessing"   )
find_project_source_files( FilesExample_Queries             "${EXAMPLE_CPP_PROJECTS_DIR}/Queries"          )
find_project_source_files( FilesExample_RenderTarget        "${EXAMPLE_CPP_PROJECTS_DIR}/RenderTarget"     )
find_project_source_files( FilesExample_ResourceBinding     "${EXAMPLE_CPP_PROJECTS_DIR}/ResourceBinding"  )
find_project_source_files( FilesExample_ShadowMapping       "${EXAMPLE_CPP_PROJECTS_DIR}/ShadowMapping"    )
find_project_source_files( FilesExample_StencilBuffer       "${EXAMPLE_CPP_PROJECTS_DIR}/StencilBuffer"    )
find_project_source_files( FilesExample_Tessellation        "${EXAMPLE_CPP_PROJECTS_DIR}/Tessellation"     )
find_project_source_files( FilesExample_Texturing           "${EXAMPLE_CPP_PROJECTS_DIR}/Texturing"        )
find_project_source_files( FilesExample_VolumeRendering     "${EXAMPLE_CPP_PROJECTS_DIR}/VolumeRendering"  )

find_project_source_files( FilesExample_HelloUWP            "${EXAMPLE_CPP_PROJECTS_DIR}/HelloUWP"         )


# === Source group folders ===

source_group("Sources" FILES ${FilesExampleBase})



# === Include directories ===

include_directories("${EXTERNAL_INCLUDE_DIR}") # <stb/stb_image.h>
include_directories("${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase")
include_directories("${SHARED_PLATFORM_DIR}")
include_directories("${GaussLib_INCLUDE_DIR}")


# === Projects ===


# Base project (static lib) for all examples
add_library(ExampleBase STATIC ${FilesExampleBaseAll})
set_target_properties(ExampleBase PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
target_link_libraries(ExampleBase "${LLGL_MODULE_LIBS}")
set_target_properties(ExampleBase PROPERTIES FOLDER "Examples")


set(EXAMPLE_PROJECT_LIBS ExampleBase ${LLGL_MODULE_LIBS})
add_llgl_example_project(Example_MultiContext   CXX "${FilesExample_MultiContext}"      "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_MultiRenderer  CXX "${FilesExample_MultiRenderer}"     "${EXAMPLE_PROJECT_LIBS}")

add_llgl_example_project(Example_Animation          CXX "${FilesExample_Animation}"         "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_ClothPhysics       CXX "${FilesExample_ClothPhysics}"      "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_IndirectDraw       CXX "${FilesExample_IndirectDraw}"      "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_Fonts              CXX "${FilesExample_Fonts}"             "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_HelloGame          CXX "${FilesExample_HelloGame}"         "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_HelloTriangle      CXX "${FilesExample_HelloTriangle}"     "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_Instancing         CXX "${FilesExample_Instancing}"        "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_MultiThreading     CXX "${FilesExample_MultiThreading}"    "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_PBR                CXX "${FilesExample_PBR}"               "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_PostProcessing     CXX "${FilesExample_PostProcessing}"    "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_Queries            CXX "${FilesExample_Queries}"           "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_RenderTarget       CXX "${FilesExample_RenderTarget}"      "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_ResourceBinding    CXX "${FilesExample_ResourceBinding}"   "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_ShadowMapping      CXX "${FilesExample_ShadowMapping}"     "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_StencilBuffer      CXX "${FilesExample_StencilBuffer}"     "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_Tessellation       CXX "${FilesExample_Tessellation}"      "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_Texturing          CXX "${FilesExample_Texturing}"         "${EXAMPLE_PROJECT_LIBS}")
add_llgl_example_project(Example_VolumeRendering    CXX "${FilesExample_VolumeRendering}"   "${EXAMPLE_PROJECT_LIBS}")




