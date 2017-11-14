#!/bin/sh

cd `dirname $0`

if [ "$JAVA_HOME" = "" ]; then JAVA_HOME=/usr/java/default; fi

COMPILE="gcc -std=gnu89 -DNDEBUG -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux -m64 -O3 -ffast-math -fweb -fomit-frame-pointer -fmerge-all-constants -fPIC -pipe -pthread -s"

yasm -f elf64 -o aes_x64.o aes_amd64.asm

echo building libaes64.so ...
$COMPILE -DASM_AMD64_C -DUSE_INTEL_AES_IF_PRESENT -shared -fvisibility=hidden -Wl,-soname -Wl,libaes64.so -o libaes64.so aes_ni.c aeskey.c aestab.c aes_x64.o

rm -f aes_x64.o 2> /dev/null
