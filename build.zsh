#!/bin/zsh

if [[ ! -d "build" ]]
then
    mkdir build
fi

cmake --build build