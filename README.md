# Install dependencies

To install all necessary dependencies there is a bash script `install_deps.sh` that will fetch all necessary 3rd-party
libraries together with skia and build it automatically. You can simply invoke it like this:

```
./install_deps.sh
```

It will build skia in parent directory. By default skia build directory is set to /out/Static in this script.

In this case you should use the same path in CMake `SKIA_BUILD` variable.

# Build:

SKIA_BUILD variable should be set as relative path to SKIA_DIR path

```
mkdir build && cd build
cmake -DSKIA_DIR=<path_to_skia_root_dir> -DSKIA_BUILD=<path_to_skia_build_dir> -DCMAKE_BUILD_TYPE=<[Release|Debug]> ..

# For example: cmake -DSKIA_DIR=/home/mateusz/dev/skia -DSKIA_BUILD=/out/Static -DCMAKE_BUILD_TYPE=Release ..

make
```

# Run:

```
./seekBarApp
```

When the app is opened you should see the gray bar. If you want to check all implemented features just drag'n'drop any file onto the window
(it will launch indeterminate loading simulation for few seconds).

After loading simulation you can check the rest of functionalities related with interview task

Note: few things like chapters relative lengths or total time on seek bar were hardcoded just to focus on interview task requirements
