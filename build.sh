set -e

objcopy -I binary -O elf64-x86-64 -B i386 fonts/Lat2-Terminus16.psfu fonts/Lat2-terminus16.o 

mkdir -p build

(
    cd build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../Toolchain.cmake ..
    cmake --build .
)

ln -sf ./build/compile_commands.json ./compile_commands.json

./make_root.sh
