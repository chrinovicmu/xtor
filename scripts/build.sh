#!/bin/bash
# build.sh - Build InstraX project from scripts folder

# Exit immediately on error
set -e

# Go to project root (assumes scripts/ is one level below root)
PROJECT_ROOT="$(dirname "$0")/.."
cd "$PROJECT_ROOT"

# Make sure build directory exists
mkdir -p build
cd build

# Run CMake configuration
echo "Configuring project..."
cmake ..

# Build with all available cores
echo "Building project..."
cmake --build . -j$(nproc)

echo "Build completed successfully!"
