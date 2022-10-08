project(LibElfIn CXX)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)

add_subdirectory(elf)
add_subdirectory(dwarf)