# Specify the target architecture
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compiler executables
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang)

set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_ASM_COMPILER_WORKS 1 CACHE INTERNAL "")

set(FREESTANDING_FLAGS "-ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -Wall -Wextra -O2 -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections -m64 -march=x86-64 -mno-sse2 -mno-sse -mno-mmx -mcmodel=kernel -mabi=sysv")

set(CMAKE_C_FLAGS_INIT "${FREESTANDING_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${FREESTANDING_FLAGS}")

set(CMAKE_ASM_FLAGS_INIT "-x assembler -masm=intel -ffreestanding -Wall -Wextra")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib -ffreestanding -O2")
