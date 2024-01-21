export CC=gcc-13
export CXX=g++-13

mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=/mnt/d/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .

