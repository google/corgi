Building for Android    {#corgi_guide_building_android}
====================

# Version Requirements

[CORGI][] is known to build with the following tool versions:

-   [Android NDK][]: android-ndk-r10e
-   [Android SDK][]: Android 5.0 (API Level 21)
-   [Python][]: 2.7.6

# Before Building

-   Install prerequisites for the developer machine's operating system.
    -   [Linux prerequisites][]
    -   [OS X prerequisites][]
    -   [Windows prerequisites][]
-   Install [fplutil prerequisites][].

# Building

To build the project:

-   Open a command line window.
-   Go to the [CORGI][] directory.
-   Run [build_all_android][] to build the project.

For example:

    cd corgi
    ./dependencies/fplutil/bin/build_all_android -E dependencies

# Installing and Running the Sample

Install the sample using [build_all_android][].

For example, the following will install and run the sample on a device attached
to the workstation with the serial number `ADA123123`.

    cd corgi
    ./dependencies/fplutil/bin/build_all_android -E dependencies -T debug -d ADA123123 -i -r

If only one device is attached to a workstation, the `-d` argument
(which selects a device) can be omitted.

<br>

  [Android]: https://www.android.com/
  [Android NDK]: http://developer.android.com/tools/sdk/ndk/index.html
  [Android SDK]: http://developer.android.com/sdk/index.html
  [build_all_android]: http://google.github.io/fplutil/build_all_android.html
  [cmake]: https://cmake.org/
  [CORGI]: @ref corgi_index
  [fplutil prerequisites]: http://google.github.io/fplutil/fplutil_prerequisites.html
  [Linux prerequisites]: @ref building_linux_prerequisites
  [OS X prerequisites]: @ref building_osx_prerequisites
  [Python]: https://www.python.org/download/releases/2.7/
  [SDK Manager]: https://developer.android.com/sdk/installing/adding-packages.html
  [WebP Precompiled Utilities]: https://developers.google.com/speed/webp/docs/precompiled
  [Windows prerequisites]: @ref building_windows_prerequisites
