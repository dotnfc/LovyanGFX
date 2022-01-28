## Get SDL2 Binary
https://www.libsdl.org/download-2.0.php
extract 'include' and 'lib', e.g. D:/temp/lovyanGFX_SDL2/sdl2, and modify the CMakeLists.txt

```bash
target_include_directories(LGFX_SDL2 PUBLIC "D:/temp/lovyanGFX_SDL2/sdl2/include")

target_link_libraries(LGFX_SDL2 PUBLIC "D:/temp/lovyanGFX_SDL2/sdl2/lib/x86/SDL2.lib" )
```

## CMake To generate Target Files
i used vscode, Ctrl+Shift+P to launch Cmake build (vs2019 x86)
compiler and linker will be involke;
or switch to vs-ide, launch build LGFX_SDL2.sln to compile & debug.

* you may need copy sdl2.dll manually to the 'build' or 'build/Debug' folder.
* 'LovyanGFX' should be in D:\temp\lovyanGFX_SDL2\LovyanGFX
 
-

----------------------------
.nfc 2022/01/28
