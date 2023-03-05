cmake_minimum_required (VERSION 3.0.0)
project (main C)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -pedantic-errors -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -g -fsanitize=address,undefined")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g")

include_directories(include)
file(GLOB_RECURSE SRC src/*.c)
add_executable(main ${SRC})
target_link_libraries(main curl)
target_link_libraries(main git2)

add_custom_target(
	copy-compile-commands ALL
	${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_BINARY_DIR}/compile_commands.json
	${CMAKE_CURRENT_LIST_DIR}
)
