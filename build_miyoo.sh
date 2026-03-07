#!/bin/bash

echo "Starting Miyoo Mini cross-compilation via Podman..."

# This runs the build inside the Miyoo toolchain container.
# It explicitly sources the setup-env.sh to get the cross-compiler in PATH,
# and passes CMAKE_CXX_COMPILER so it doesn't accidentally use the x86 host compiler.
podman run --user root -v "$(pwd)":/root/workspace:Z miyoomini-toolchain /bin/bash -c "
    echo 'Cleaning old build directory...' &&
    rm -rf /root/workspace/build_miyoo_clean &&
    
    echo 'Setting up environment...' &&
    . /root/setup-env.sh &&
    
    echo 'Configuring CMake...' &&
    cmake -B /root/workspace/build_miyoo_clean -S /root/workspace \\
          -DMIYOO_BUILD=ON \\
          -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \\
          -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ &&
          
    echo 'Building project...' &&
    cmake --build /root/workspace/build_miyoo_clean -j\$(nproc) &&
    
    echo 'Fixing file permissions...' &&
    chown -R 1000:1000 /root/workspace/build_miyoo_clean
"

if [ $? -eq 0 ]; then
    echo "Build completed successfully! Binary is located in ./build_miyoo_clean/main"
else
    echo "Build failed!"
    exit 1
fi
