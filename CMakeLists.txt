cmake_minimum_required(VERSION 4.0)
project(ktkrnl LANGUAGES C CXX ASM)

add_compile_options(-fno-pic -fno-pie -mcmodel=kernel)
add_link_options(-no-pie)

if (NOT CMAKE_TOOLCHAIN_FILE)
  message(FATAL "No toolchain file provided")
endif()

file(GLOB_RECURSE font_objects
  fonts/**.o
)

file(GLOB_RECURSE ktcore_sources
  ktcore/**.cpp
  ktcore/**.c
  ktcore/**.h
  ktcore/**.hpp
  ktcore/**.s
)

add_library(ktcore STATIC ${ktcore_sources})
target_include_directories(ktcore PRIVATE ktcore/)

file(GLOB_RECURSE ktkernel_sources
  ktkernel/**.cpp
  ktkernel/**.c
  ktkernel/**.h
  ktkernel/**.hpp
  ktkernel/**.s
)

add_executable(ktkernel ${ktkernel_sources} ${font_objects})
target_link_libraries(ktkernel PRIVATE ktcore)
target_include_directories(ktkernel PRIVATE ktkernel/ ktcore/)
target_link_options(ktkernel PRIVATE -T${CMAKE_SOURCE_DIR}/link.ld)
