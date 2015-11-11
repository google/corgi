Building for OS X    {#corgi_guide_building_osx}
=================

You can use [cmake][] to generate an [Xcode][] project for [CORGI][] on
[OS X][].

# Version Requirements

[CORGI][] is known to build with the following tool versions:

-   [cmake][]: 2.8.12
-   [OS X][]: Yosemite (10.10.1)
-   [Xcode][]: 6.4

# Before Building    {#building_osx_prerequisites}

-   Install [Xcode][].
    -   Be sure to open up `Xcode` after installing to accept the license.
-   Install [cmake][].
    -   Add the directory containing [cmake][]'s `bin` folder to the `PATH`
        variable.
        -   For example, if [cmake][] is installed in
            `/Applications/CMake.app`, the following line should be added to
            the user's bash resource file `~/.bash_profile`:<br>
            `export PATH="$PATH:/Applications/CMake.app/Contents/bin"`

# Creating the Xcode Project

The [Xcode][] project is generated using [cmake][].

For example, the following generates the [Xcode][] project in the `corgi`
directory.

~~~{.sh}
    cd corgi
    cmake -G "Xcode"
~~~

# Building with Xcode

-   Open the `corgi/corgi.xcodeproj` project using [Xcode][] from the command
    line:
~~~{.sh}
   cd corgi
   open corgi.xcodeproj
~~~
    It's important to open the project from the command line so the modified
    `PATH` variable is available in [Xcode][].  Failure to launch from the
    command line can result in build errors due to `Xcode` being unable to find
    tools. An alternative would be to either install tools in
    system bin directories (e.g /usr/bin) or modify the system wide `PATH`
    variable (using [environment.plist][]) to include paths to all required
    tools.
-   Select `Product-->Build` from the menu.

You can also build the sample from the command-line.

-   Run `xcodebuild` after generating the Xcode project to build all targets.
    -   You may need to force the `generated_includes` target to be built first.

For example, in the `corgi` directory:

~~~{.sh}
    xcodebuild -target generated_includes
    xcodebuild
~~~

# Executing the Sample with Xcode

-   Select `entity_component_system_sample` as the `Scheme`, for example
    `entity_component_system_sample-->My Mac`, from the combo box to the right
    of the `Run` button.
-   Click the `Run` button.

You can also run the sample from the command-line.

For example:

    ./bin/Debug/entity_component_system_sample

<br>

  [cmake]: http://www.cmake.org
  [CORGI]: @ref corgi_index
  [OS X]: http://www.apple.com/osx/
  [Xcode]: http://developer.apple.com/xcode/
  [environment.plist]: https://developer.apple.com/library/mac/documentation/MacOSX/Conceptual/BPRuntimeConfig/Articles/EnvironmentVars.html
