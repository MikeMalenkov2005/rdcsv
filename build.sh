#!/bin/bash

cmake -S . -B build
cmake --build build --config Release
cmake --install build --config Release --prefix .

