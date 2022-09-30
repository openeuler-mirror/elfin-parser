project(LibElfInElf CXX)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_CURRENT_LIST_DIR}/../Common.cmake)

link_directories(${CMAKE_BINARY_DIR}/lib)
file(GLOB elf++_src CONFIGURE_DEPENDS "*.cc" "*.hh")
add_library(elf++ STATIC ${elf++_src})

set_target_properties(elf++ PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib64"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
