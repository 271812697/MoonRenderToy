cmake_minimum_required(VERSION 3.23)
set(TARGET encode)
set(SRCS
	src/main.cpp
	src/Path.h
	src/Path.cpp
)

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET} PUBLIC std_image)
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)







