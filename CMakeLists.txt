cmake_minimum_required(VERSION 4.0)
project(ktkrnl LANGUAGES C CXX ASM)

add_compile_options(-fno-pic -fno-pie -mcmodel=kernel)
add_link_options(-no-pie)

if (NOT CMAKE_TOOLCHAIN_FILE)
  message(FATAL "No toolchain file provided")
endif()

#file(GLOB_RECURSE font_objects
#  fonts/**.o
#)

include(cmake/ModuleCompiler.cmake)

add_kernel_module(cmos
    SOURCES modules/cmos/cmos.cpp
    INCLUDES kt/ modules/cmos/
    #COMPILE_FLAGS -O2
)

file(GLOB_RECURSE ktcore_sources
  ktcore/**.cpp
  ktcore/**.c
  ktcore/**.h
  ktcore/**.hpp
  ktcore/**.s
)

add_library(ktcore STATIC ${ktcore_sources})
target_include_directories(ktcore PRIVATE ktcore/ kt/)

file(GLOB_RECURSE ktkernel_sources
  ktkernel/**.cpp
  ktkernel/**.c
  ktkernel/**.h
  ktkernel/**.hpp
  ktkernel/**.s
)

add_executable(ktkernel ${ktkernel_sources})
target_compile_definitions(ktkernel PRIVATE QEMU)
target_link_libraries(ktkernel PRIVATE -Wl,--whole-archive ktcore -Wl,--no-whole-archive)
target_include_directories(ktkernel PRIVATE ktkernel/ ktcore/ kt/)
target_link_options(ktkernel PRIVATE -T${CMAKE_SOURCE_DIR}/link.ld)

modulec_merge_compdb()
