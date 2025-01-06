#!/bin/bash

# Function to check for errors and exit if necessary
check_error() {
    if [ $? -ne 0 ]; then
        echo "An error occurred. Exiting."
        exit 1
    fi
}

# Step 1: Set up variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKIA_PARENT_DIR="$(dirname "$SCRIPT_DIR")"
SKIA_DIR="$SKIA_PARENT_DIR/skia"
BUILD_DIR="$SKIA_DIR/out/Static"
DEPOT_TOOLS_DIR="$SKIA_PARENT_DIR/depot_tools"

echo "Skia build script started..."
echo "Skia directory: $SKIA_DIR"
echo "Depot tools directory: $DEPOT_TOOLS_DIR"

# Step 2: Install dependencies
echo "Installing dependencies..."
sudo apt-get update
sudo apt-get install -y python3 python3-pip curl git clang cmake ninja-build libfontconfig1-dev libfreetype6-dev libjpeg-dev libwebp-dev libicu-dev libglfw3-dev
check_error

# Step 3: Clone depot_tools if not already cloned
if [ ! -d "$DEPOT_TOOLS_DIR" ]; then
    echo "Cloning depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git "$DEPOT_TOOLS_DIR"
    check_error
else
    echo "Depot tools already exists. Skipping clone."
fi

# Add depot_tools to PATH
export PATH="$DEPOT_TOOLS_DIR:$PATH"

# Step 4: Clone Skia if not already cloned
if [ ! -d "$SKIA_DIR" ]; then
    echo "Cloning Skia repository..."
    git clone https://github.com/google/skia.git "$SKIA_DIR"
    check_error
else
    echo "Skia directory already exists. Skipping clone."
fi

cd "$SKIA_DIR" || exit

# Step 5: Sync Skia dependencies
echo "Syncing Skia dependencies..."
python3 tools/git-sync-deps --jobs=1
check_error

# Step 6: Generate build files
echo "Generating build files..."
mkdir -p "$BUILD_DIR"
bin/gn gen "$BUILD_DIR" --args='is_official_build=true skia_use_system_harfbuzz=false cc="clang" cxx="clang++"'
check_error

# Step 7: Build Skia
echo "Building Skia using ninja..."
ninja -C "$BUILD_DIR" skia 2>&1
check_error

echo "Skia build completed successfully!"
echo "Build output is located in: $BUILD_DIR"
