project(LibElfInDwarf CXX)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_CURRENT_LIST_DIR}/../Common.cmake)

link_directories(${CMAKE_BINARY_DIR}/lib)
file(GLOB libelfin_dwarf_src CONFIGURE_DEPENDS "*.cc" "*.hh")
add_library(dwarf++ STATIC ${libelfin_dwarf_src})
add_dependencies(dwarf++ elf++)

set_target_properties(dwarf++ PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib64"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
