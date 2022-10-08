cmake_minimum_required(VERSION 3.12)

include_guard(GLOBAL)

set(compiler "gcc")
if (NOT "${CMAKE_C_COMPILER}" STREQUAL "")
    set(compiler "${CMAKE_C_COMPILER}")
endif()

find_file(compiler_full_path "${compiler}")
get_filename_component(compiler_dir "${compiler_full_path}" DIRECTORY)

file(GLOB cxx_libs "${compiler_dir}/../lib64/libstdc++.so*")
list(FILTER cxx_libs INCLUDE REGEX "libstdc..\.so(\.[0-9]+)?(\.[0-9]+)?(\.[0-9]+)?$")
install(PROGRAMS ${cxx_libs}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib64
)