cmake_minimum_required (VERSION 3.8)

# Set application to GUI type for Win32 and MacOS
if(WIN32)
  set(GUI_TYPE WIN32)
endif(WIN32)
if(APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
endif(APPLE)

project(LGFX_SDL2)

ADD_DEFINITIONS(-DTARGET_SDL2)
ADD_DEFINITIONS(-D_CONSOLE)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS 
    *.cpp
    # ../../../../LovyanGFX/src/lgfx/Fonts/efont/*.c
    # ../../../../LovyanGFX/src/lgfx/Fonts/IPA/*.c
    ../../../../LovyanGFX/src/lgfx/utility/*.c
    ../../../../LovyanGFX/src/lgfx/v1/*.cpp
    ../../../../LovyanGFX/src/lgfx/v1/misc/*.cpp
    ../../../../LovyanGFX/src/lgfx/v1/panel/*.cpp
    ../../../../LovyanGFX/src/lgfx/v1/platforms/sdl2/*.cpp
    )
add_executable (LGFX_SDL2 ${Target_Files})

target_compile_features(LGFX_SDL2 PUBLIC cxx_std_17)
target_include_directories(LGFX_SDL2 PUBLIC "../../../../LovyanGFX/src/")
target_include_directories(LGFX_SDL2 PUBLIC "D:/temp/lovyanGFX_SDL2/sdl2/include")
target_link_libraries(LGFX_SDL2 PUBLIC "D:/temp/lovyanGFX_SDL2/sdl2/lib/x86/SDL2.lib" )
