#!/bin/bash

g++ -o main src/*.cpp --std=c++17 main.cpp -I include
./main.exe

exec bash