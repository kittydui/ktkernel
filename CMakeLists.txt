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

file(GLOB_RECURSE ktkernel_sources
  ktkernel/**.cpp
  ktkernel/**.c
  ktkernel/**.s
)

add_executable(ktkernel ${ktkernel_sources})
target_include_directories(ktkernel PRIVATE ktkernel/ kt/)
target_link_options(ktkernel PRIVATE -T${CMAKE_SOURCE_DIR}/misc/link.ld)

modulec_merge_compdb()
