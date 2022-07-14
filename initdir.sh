#!/bin/bash
mkdir "./images/$1"
tar -C "./images/$1" -zxvf rootfs.tar.gz

echo "created dir!!!"