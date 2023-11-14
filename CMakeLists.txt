cmake_minimum_required(VERSION 3.24)
project(knr_exercises C)

set(CMAKE_C_STANDARD 11)

add_executable(exercise1_24 "1-24/main.c")
add_custom_command(
    TARGET exercise1_24 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/1-24/test files" "$<TARGET_FILE_DIR:exercise1_24>/test files"
)

add_executable(exercise1_20 "1-20/main.c")
add_custom_command(
    TARGET exercise1_20 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/1-20/test files" "$<TARGET_FILE_DIR:exercise1_20>/test files"
)
