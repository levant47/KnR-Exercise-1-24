cmake_minimum_required(VERSION 3.24)
project(olegloh C)

set(CMAKE_C_STANDARD 11)

add_executable(olegloh
    main.c)

add_custom_command(
    TARGET olegloh POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/test files" "$<TARGET_FILE_DIR:olegloh>/test files"
)
