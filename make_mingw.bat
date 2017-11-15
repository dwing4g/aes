@echo off
setlocal
pushd %~dp0

rem install mingw-gcc 4.8+ and yasm
rem change aesopt.h: "#if 1 && defined( VIA_ACE_POSSIBLE ) && !defined( USE_VIA_ACE_IF_PRESENT )" to "#if 0 ..."

set MINGW_HOME=C:\mingw
set MINGW_BIN=%MINGW_HOME%\bin

set COMPILE=-std=gnu89 -DNDEBUG -DWIN32 -D_WINDOWS -D_USRDLL -DDLL_EXPORT -DENABLE_JNI -Ijni -Ofast -ffast-math -fweb -fomit-frame-pointer -fmerge-all-constants -flto -fwhole-program -pipe -static -s

set COMPILE32=%MINGW_BIN%\i686-w64-mingw32-gcc.exe -m32 -march=i686 %COMPILE%
set COMPILE64=%MINGW_BIN%\x86_64-w64-mingw32-gcc.exe -m64 %COMPILE%

yasm -f win32 -d ASM_X86_V2 -o aes_x86.obj aes_x86_v2.asm
yasm -f win64 -o aes_x64.obj aes_amd64.asm

echo building aesjni32.dll ...
%COMPILE32% -DASM_X86_V2 -shared -Wl,--enable-stdcall-fixup -Wl,--image-base,0x10000000 -Wl,--kill-at -Wl,--compat-implib -Wl,--out-implib,aesjni32.lib -Wl,-soname -Wl,aesjni32.dll -o aesjni32.dll jni.c aes_x86.obj aesjni32.def

echo building aesjni64.dll ...
%COMPILE64% -DASM_AMD64_C -DUSE_INTEL_AES_IF_PRESENT -shared -Wl,--image-base,0x10000000 -Wl,--kill-at -Wl,--compat-implib -Wl,--out-implib,aesjni64.lib -Wl,-soname -Wl,aesjni64.dll -o aesjni64.dll aes_ni.c aeskey.c aestab.c jni.c aes_x64.obj aesjni64.def

echo building aestest32.exe ...
%COMPILE32% -DASM_X86_V2 -static -o aestest32.exe aestest.c aes_x86.obj

echo building aestest64.exe ...
%COMPILE64% -DASM_AMD64_C -DUSE_INTEL_AES_IF_PRESENT -static -o aestest64.exe aes_ni.c aeskey.c aestab.c aestest.c aes_x64.obj

del aes_x86.obj
del aes_x64.obj

pause
