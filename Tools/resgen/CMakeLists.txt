cmake_minimum_required(VERSION 3.19)

project(resgen)



set(TARGET resgen)



# ==================================================================================================

# Source files

# ==================================================================================================

set(SRCS

	src/main.cpp

	src/Path.h

	src/Path.cpp

)

# ==================================================================================================

# Target definitions

# ==================================================================================================

add_executable(${TARGET} ${SRCS})

set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

