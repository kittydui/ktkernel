set -e 

mkdir -p build

(
    cd build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain.cmake ..
    cmake --build .
)

ln -sf ./build/compile_commands.json ./compile_commands.json

./scripts/make_root.sh
