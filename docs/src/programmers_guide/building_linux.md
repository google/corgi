Building for Linux    {#corgi_guide_building_linux}
==================

# Version Requirements

[CORGI][] is known to build with the following tool versions:

-   [cmake][]: 2.8.12

# Before Building    {#building_linux_prerequisites}

Prior to building, install the following components using the [Linux][]
distribution's package manager:

-    [cmake][] (You can also manually install from [cmake.org][].)

For example, on Ubuntu:

    sudo apt-get install cmake

# Building

-   Generate makefiles from the [cmake][] project in the `corgi` directory.
-   Execute `make` to build the library and sample.

For example:

    cd corgi
    cmake -G'Unix Makefiles'
    make

To perform a debug build:

    cd corgi
    cmake -G'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug
    make

Build targets can be configured using options exposed in
`corgi/CMakeLists.txt` by using cmake's `-D` option.
Build configuration set using the `-D` option is sticky across subsequent
builds.

For example, if a build is performed using:

    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
    make

to switch to a release build CMAKE_BUILD_TYPE must be explicitly specified:

    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
    make

# Executing the Sample

After building the project, you can execute the sample from the command line.
For example:

    ./bin/entity_component_system_sample

<br>

  [cmake]: http://www.cmake.org/
  [cmake.org]: http://www.cmake.org/
  [CORGI]: @ref corgi_index
  [Linux]: http://en.wikipedia.org/wiki/Linux
