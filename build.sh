#!/usr/bin/env bash

set -e

# cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
