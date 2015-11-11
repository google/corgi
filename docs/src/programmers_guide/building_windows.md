Building for Windows    {#corgi_guide_building_windows}
====================

You can use [cmake][] to generate a [Visual Studio][] project for [CORGI][]
on [Windows][].

# Version Requirements

[CORGI][] is known to build with the following tool versions:

-   [cmake][]: 2.8.12
-   [Visual Studio][]: 2010 or 2012
-   [Windows][]: 7

# Before Building    {#building_windows_prerequisites}

Use [cmake][] to generate the [Visual Studio][] solution and project files.

The following example generates the [Visual Studio][] 2012 solution in the
`corgi` directory:

    cd corgi
    cmake -G "Visual Studio 11"

To generate a [Visual Studio][] 2010 solution, use this command:

    cd corgi
    cmake -G "Visual Studio 10"

Running [cmake][] under [cygwin][] requires empty `TMP`, `TEMP`, `tmp` and
`temp` variables. To generate a [Visual Studio][] solution from a [cygwin][]
bash shell use:

    $ cd corgi
    $ ( unset {temp,tmp,TEMP,TMP} ; cmake -G "Visual Studio 11" )


# Building with Visual Studio

-   Double-click on `corgi/corgi.sln` to open the solution.
-   Select `Build-->Build Solution` from the menu.

It's also possible to build from the command line using `msbuild` after using
`vsvars32.bat` to set up the [Visual Studio][] build environment. For example,
assuming [Visual Studio][] is installed in
`c:\Program Files (x86)\Microsoft Visual Studio 11.0`:

    cd corgi
    "c:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\vsvars32.bat"
    cmake -G "Visual Studio 11"
    msbuild corgi.sln

# Executing the Sample with Visual Studio

-   Right-click on the `corgi` project in the Solution Explorer
    pane, and select `Set as Startup Project`.
-   Select `Debug-->Start Debugging` from the menu.

<br>

  [cmake]: http://www.cmake.org
  [CORGI]: @ref corgi_index
  [cygwin]: https://www.cygwin.com/
  [Visual Studio]: http://www.visualstudio.com/
  [Windows]: http://windows.microsoft.com/
