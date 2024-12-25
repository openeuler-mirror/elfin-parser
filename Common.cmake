cmake_minimum_required(VERSION 3.12)

include_guard(DIRECTORY)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fvisibility=hidden -fdiagnostics-color=always -march=armv8.2-a -mcpu=tsv110 -mno-outline-atomics")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

if (NOT FORCE_DEFAULT_INSTALL_PREFIX AND "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    message(FATAL_ERROR "Using standard install prefix is dangerous. Please set -DCMAKE_INSTALL_PREFIX to non default location or use -DFORCE_DEFAULT_INSTALL_PREFIX=1.")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/InstallStdcxx.cmake)
