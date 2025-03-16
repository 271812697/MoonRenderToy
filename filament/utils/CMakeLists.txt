cmake_minimum_required(VERSION 3.19)
set(TARGET utils)
set(TARGET_GENERIC ${TARGET}/generic)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

set(DIST_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET}/algorithm.h
        ${PUBLIC_HDR_DIR}/${TARGET}/bitset.h
        ${PUBLIC_HDR_DIR}/${TARGET}/CallStack.h
        ${PUBLIC_HDR_DIR}/${TARGET}/debug.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Allocator.h
        ${PUBLIC_HDR_DIR}/${TARGET}/BitmaskEnum.h
        ${PUBLIC_HDR_DIR}/${TARGET}/compiler.h
        ${PUBLIC_HDR_DIR}/${TARGET}/compressed_pair.h
        ${PUBLIC_HDR_DIR}/${TARGET}/CString.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Entity.h
        ${PUBLIC_HDR_DIR}/${TARGET}/EntityInstance.h
        ${PUBLIC_HDR_DIR}/${TARGET}/EntityManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/FixedCapacityVector.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Invocable.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Log.h
        ${PUBLIC_HDR_DIR}/${TARGET}/memalign.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Mutex.h
        ${PUBLIC_HDR_DIR}/${TARGET}/NameComponentManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/ostream.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Panic.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Path.h
        ${PUBLIC_HDR_DIR}/${TARGET}/PrivateImplementation.h
        ${PUBLIC_HDR_DIR}/${TARGET}/PrivateImplementation-impl.h
        ${PUBLIC_HDR_DIR}/${TARGET}/SingleInstanceComponentManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Slice.h
        ${PUBLIC_HDR_DIR}/${TARGET}/StructureOfArrays.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Systrace.h
        ${PUBLIC_HDR_DIR}/${TARGET}/sstream.h
        ${PUBLIC_HDR_DIR}/${TARGET}/unwindows.h
)



set(DIST_GENERIC_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET_GENERIC}/Mutex.h
)

set(SRCS
        src/api_level.cpp
        src/architecture.cpp
        src/ashmem.cpp
        src/debug.cpp
        src/Allocator.cpp
        src/CallStack.cpp
        src/CString.cpp
        src/CountDownLatch.cpp
        src/CyclicBarrier.cpp
        src/EntityManager.cpp
        src/EntityManagerImpl.h
        src/JobSystem.cpp
        src/Log.cpp
        src/NameComponentManager.cpp
        src/ostream.cpp
        src/Panic.cpp
        src/Path.cpp
        src/Profiler.cpp
        src/sstream.cpp
        src/string.cpp
        src/ThreadUtils.cpp
)

list(APPEND SRCS src/win32/Path.cpp)
# ==================================================================================================
# Includes and target definition
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR} ${Robin_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
target_link_libraries(${TARGET} PUBLIC tsl)
 # Needed for shlwapi.h (GetModuleFileName)
 target_link_libraries(${TARGET} PUBLIC Shlwapi)

