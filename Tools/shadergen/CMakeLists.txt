cmake_minimum_required(VERSION 3.23)
set(TARGET shadergen)
set(SRCS
	src/main.cpp
	src/Path.h
	src/Path.cpp
)
add_executable(${TARGET} ${SRCS})
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)







