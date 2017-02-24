CORGI Version 1.0.2    {#corgi_readme}
===================

CORGI is a C++ [entity-component system][] library developed primarily for
games that focus on simplicity and flexibility.

The core functionality of CORGI is provided by the following classes:
   * Component
      - An object that contains the logic and data pertaining to a particular
        system in the game.
   * Entity
      - The basic building block of a game, that does not do much on its own. It
        can be associated with many Components to achieve more complex behavior.
   * Entity Manager
      - This is a the object that ties Entities and Components together. It acts
        as the main point of interface for game logic to create Entities and
        register them with Components.

The library is written in portable C++ and has been tested on the following
platforms:

   * [Android][]
   * [Linux][] (x86_64)
   * [OS X][]
   * [WIndows][]

Go to our [landing page][] to browse our documentation and see some examples.

   * Discuss CORGI with other developers and users on the
     [CORGI Google Group][].
   * File issues on the [CORGI Issues Tracker][].
   * Post your questoins to [stackoverflow.com][] with a mention of
     **fpl corgi**.

**Important**: The CORGI component library uses submodules, so download the
source using:

~~~{.sh}
  git clone --recursive https://github.com/google/corgi.git
~~~

To contribute to this project see [CONTRIBUTING][].

<br>

   [Android]: http://www.android.com
   [CONTRIBUTING]: http://github.com/google/corgi/blob/master/CONTRIBUTING
   [CORGI Google Group]: http://groups.google.com/group/corgi-lib
   [CORGI Issues Tracker]: http://github.com/google/corgi/issues
   [entity-component system]: https://en.wikipedia.org/wiki/Entity_component_system
   [landing page]: http://google.github.io/corgi
   [Linux]: http://en.m.wikipedia.org/wiki/Linux
   [OS X]: http://www.apple.com/osx/
   [stackoverflow.com]: http://stackoverflow.com/search?q=fpl+corgi
   [Windows]: http://windows.microsoft.com/
