cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(Patchwork_vita)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline -I libraries/tomlc17/src")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(Patchwork_vita
  src/main.c
  src/clib.c
  src/sha256.c
  src/hooks.c
  src/badtoml/toml.c
  src/badtoml/keymap.c
  src/badtoml/tokenizer.c
  src/config.c
)

target_link_libraries(Patchwork_vita
  taihen_stub
  SceLibKernel_stub
  SceDebugForDriver_stub
  SceKernelUtilsForDriver_stub
  SceIofilemgr_stub
)

set_target_properties(Patchwork_vita
  PROPERTIES LINK_FLAGS "-nostartfiles"
)

vita_create_self(patchwork.suprx Patchwork_vita CONFIG exports.yml UNSAFE)