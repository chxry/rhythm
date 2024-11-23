# Configure with [fxsdk build-fx] or [fxsdk build-cg], which provide the
# toolchain file and module path of the fxSDK

cmake_minimum_required(VERSION 3.15)
project(Rhythm)

include(GenerateG3A)
include(Fxconv)
find_package(Gint 2.9 REQUIRED)

set(SOURCES
  src/main.c
)
set(ASSETS
  assets/miku.png
)

fxconv_declare_assets(${ASSETS} WITH_METADATA)

add_executable(rhythm ${SOURCES} ${ASSETS})
target_compile_options(rhythm PRIVATE -Wall -Wextra -Os -g -flto)
target_link_libraries(rhythm Gint::Gint)

generate_g3a(TARGET rhythm OUTPUT "rhythm.g3a"
  NAME "rhythm" ICONS assets/icon.png assets/icon.png)
