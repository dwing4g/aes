@echo off
setlocal
pushd %~dp0

rem install mingw-gcc 4.8+ and yasm
rem change aesopt.h: "#if 1 && defined( VIA_ACE_POSSIBLE ) && !defined( USE_VIA_ACE_IF_PRESENT )" to "#if 0 ..."

set MINGW_HOME=C:\mingw
set MINGW_BIN=%MINGW_HOME%\bin

set COMPILE=-std=gnu89 -DNDEBUG -DWIN32 -D_WINDOWS -Ofast -ffast-math -fweb -fomit-frame-pointer -fmerge-all-constants -flto -fwhole-program -pipe -static -s

set COMPILE32=%MINGW_BIN%\i686-w64-mingw32-gcc.exe -m32 -march=i686 %COMPILE%
set COMPILE64=%MINGW_BIN%\x86_64-w64-mingw32-gcc.exe -m64 %COMPILE%

yasm -f win32 -d ASM_X86_V2C -o aes_x86.obj aes_x86_v2.asm
yasm -f win64 -o aes_x64.obj aes_amd64.asm

echo building aes32.dll ...
%COMPILE32% -D_USRDLL -DDLL_EXPORT -DASM_X86_V2C -shared -Wl,--image-base,0x10000000 -Wl,--kill-at -Wl,--compat-implib -Wl,--out-implib,aes32.lib -Wl,--output-def,aes32.def -Wl,-soname -Wl,aes32.dll -o aes32.dll aeskey.c aestab.c aes_x86.obj

echo building aes64.dll ...
%COMPILE64% -D_USRDLL -DDLL_EXPORT -DASM_AMD64_C -DUSE_INTEL_AES_IF_PRESENT -shared -Wl,--image-base,0x10000000 -Wl,--kill-at -Wl,--compat-implib -Wl,--out-implib,aes64.lib -Wl,--output-def,aes64.def -Wl,-soname -Wl,aes64.dll -o aes64.dll aes_ni.c aeskey.c aestab.c aes_x64.obj

del aes_x86.obj
del aes_x64.obj

pause
