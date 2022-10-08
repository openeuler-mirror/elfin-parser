#!/bin/bash
if [ -d _build ]; then
    rm -rf _build
fi

INSTALL_PREFIX=install
if [ -n "$1" ]; then
    INSTALL_PREFIX=$1
    shift
fi

BUILD_TYPE=Release
if [ -n "$1" ]; then
    BUILD_TYPE=$1
    shift
fi

cmake -S . -B _build -DCMAKE_INSTALL_PREFIX=$(realpath ${INSTALL_PREFIX}) -DCMAKE_BUILD_TYPE=$BUILD_TYPE $@ 2>&1 | tee cmake.log
EXIT_CODE=${PIPESTATUS[0]}
if [ $EXIT_CODE -ne 0 ]; then
    exit ${EXIT_CODE}
fi
cmake --build _build --parallel -v 2>&1 | tee make.log
EXIT_CODE=${PIPESTATUS[0]}
if [ $EXIT_CODE -ne 0 ]; then
    exit ${EXIT_CODE}
fi

cmake --install _build 2>&1 | tee install.log
exit ${PIPESTATUS[0]}