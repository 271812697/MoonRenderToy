cmake_minimum_required(VERSION 3.19)
project(filament C ASM)

set(TARGET backend)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/backend/AcquiredImage.h
        include/backend/BufferDescriptor.h
        include/backend/CallbackHandler.h
        include/backend/DescriptorSetOffsetArray.h
        include/backend/DriverApiForward.h
        include/backend/DriverEnums.h
        include/backend/Handle.h
        include/backend/PipelineState.h
        include/backend/PixelBufferDescriptor.h
        include/backend/Platform.h
        include/backend/Program.h
        include/backend/SamplerDescriptor.h
        include/backend/TargetBufferInfo.h
)

set(SRCS
        src/BackendUtils.cpp
        src/BlobCacheKey.cpp
        src/Callable.cpp
        src/CallbackManager.cpp
        src/CircularBuffer.cpp
        src/CommandBufferQueue.cpp
        src/CommandStream.cpp
        src/CompilerThreadPool.cpp
        src/Driver.cpp
        src/Handle.cpp
        src/HandleAllocator.cpp
        src/ostream.cpp
        src/noop/NoopDriver.cpp
        src/noop/PlatformNoop.cpp
        src/Platform.cpp
        src/PlatformFactory.cpp
        src/Program.cpp
        src/SamplerGroup.cpp
)

set(PRIVATE_HDRS
        include/private/backend/CircularBuffer.h
        include/private/backend/CommandBufferQueue.h
        include/private/backend/CommandStream.h
        include/private/backend/Dispatcher.h
        include/private/backend/Driver.h
        include/private/backend/DriverApi.h
        include/private/backend/DriverAPI.inc
        include/private/backend/HandleAllocator.h
        include/private/backend/PlatformFactory.h
        include/private/backend/SamplerGroup.h
        src/CallbackManager.h
        src/CommandStreamDispatcher.h
        src/CompilerThreadPool.h
        src/DataReshaper.h
        src/DriverBase.h
)

# ==================================================================================================
# OpenGL / OpenGL ES Sources
# ==================================================================================================


    list(APPEND SRCS
            include/backend/platforms/OpenGLPlatform.h
            src/opengl/BindingMap.h
            src/opengl/gl_headers.cpp
            src/opengl/gl_headers.h
            src/opengl/GLBufferObject.h
            src/opengl/GLDescriptorSet.cpp
            src/opengl/GLDescriptorSet.h
            src/opengl/GLDescriptorSetLayout.h
            src/opengl/GLTexture.h
            src/opengl/GLUtils.cpp
            src/opengl/GLUtils.h
            src/opengl/OpenGLBlobCache.cpp
            src/opengl/OpenGLBlobCache.h
            src/opengl/OpenGLContext.cpp
            src/opengl/OpenGLContext.h
            src/opengl/OpenGLDriver.cpp
            src/opengl/OpenGLDriver.h
            src/opengl/OpenGLDriverFactory.h
            src/opengl/OpenGLProgram.cpp
            src/opengl/OpenGLProgram.h
            src/opengl/OpenGLPlatform.cpp
            src/opengl/OpenGLTimerQuery.cpp
            src/opengl/OpenGLTimerQuery.h
            src/opengl/ShaderCompilerService.cpp
            src/opengl/ShaderCompilerService.h
    )
    if (EGL)
        list(APPEND SRCS src/opengl/platforms/PlatformEGL.cpp)
    endif()

        list(APPEND SRCS src/opengl/platforms/PlatformWGL.cpp)





# ==================================================================================================
# Definitions
# ==================================================================================================
# "2" corresponds to SYSTRACE_TAG_FILEMENT (See: utils/Systrace.h)
add_definitions(-DSYSTRACE_TAG=2 )

# ==================================================================================================
# Includes & target definition
# ==================================================================================================
# specify where our headers are
include_directories(${PUBLIC_HDR_DIR})
include_directories(src)
include_directories(${GENERATION_ROOT})

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# add this subproject to the Filament folder
set_target_properties(${TARGET} PROPERTIES FOLDER Filament)

# ==================================================================================================
# Expose a header-only target to minimize dependencies.
# ==================================================================================================

add_library(${TARGET}_headers INTERFACE)
target_include_directories(${TARGET}_headers INTERFACE ${PUBLIC_HDR_DIR})



# ==================================================================================================
# Dependencies
# ==================================================================================================

if (ANDROID)
    if (FILAMENT_SUPPORTS_OPENGL)
        target_link_libraries(${TARGET} PUBLIC GLESv3 EGL)
    endif()
    target_link_libraries(${TARGET} PUBLIC android)
endif()

if (FILAMENT_USE_SWIFTSHADER)
    target_link_libraries(${TARGET} PUBLIC ${SWIFTSHADER_VK})
endif()

if (APPLE AND NOT IOS)
    target_link_libraries(${TARGET} PRIVATE "-framework Cocoa")
endif()

target_link_libraries(${TARGET} PUBLIC Maths)
target_link_libraries(${TARGET} PUBLIC utils)


target_link_libraries(${TARGET} PRIVATE bluegl)







# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    set(OPTIMIZATION_FLAGS
        /fp:fast
    )
elseif(WEBGL)
    # Avoid strict-vtable-pointers here, it is broken in WebAssembly.
    set(OPTIMIZATION_FLAGS -fvisibility-inlines-hidden)
else()
    set(OPTIMIZATION_FLAGS
        -ffast-math
        -fno-finite-math-only
        -ffp-contract=fast
        # TODO: aggressive vectorization is currently broken on Android
        #        -fslp-vectorize-aggressive
        -fvisibility-inlines-hidden
        -fstrict-vtable-pointers
    )
endif()



if (MSVC)
    set(FILAMENT_WARNINGS /W3)
else()
    set(FILAMENT_WARNINGS
            -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
            -Wextra-semi -Wnewline-eof -Wdeprecated -Wundef
            -Wgnu-conditional-omitted-operand
            -Wweak-vtables -Wnon-virtual-dtor -Wclass-varargs -Wimplicit-fallthrough
            -Wover-aligned
            -Werror
    )
    if (CMAKE_CXX_STANDARD EQUAL 20)
        list(APPEND FILAMENT_WARNINGS -Wno-deprecated-this-capture)
    endif()
endif()



target_compile_options(${TARGET} PRIVATE
        ${FILAMENT_WARNINGS}
        ${OSMESA_COMPILE_FLAGS}
        $<$<CONFIG:Release>:${OPTIMIZATION_FLAGS}>
        $<$<AND:$<PLATFORM_ID:Darwin>,$<CONFIG:Release>>:${DARWIN_OPTIMIZATION_FLAGS}>
)


target_link_libraries(${TARGET} PRIVATE
        ${OSMESA_LINKER_FLAGS}
        $<$<AND:$<PLATFORM_ID:Linux>,$<CONFIG:Release>>:${LINUX_LINKER_OPTIMIZATION_FLAGS}>
)

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/backend DESTINATION include)


