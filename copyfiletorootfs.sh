#!/bin/bash
echo "moving testcases $1"
g++ client.cpp -o client
sudo cp server.cpp ./images/$1/rootfs
sudo cp memoryIntensive.cpp ./images/$1/rootfs
