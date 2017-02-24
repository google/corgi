CORGI
=====

# Overview    {#corgi_index}

[CORGI][] is a C++ [entity-component system][] library developed primarily for
games that focus on simplicity and flexibility.

It provides an [Entity Manager][] class, which manages all the [Entity][] and
[Component][] classes used by your game. [CORGI][] also provides a
[Component Library][], which contains a suite of common game-service Components
for your use.

[CORGI][] can be downloaded from [GitHub][] or the [releases page][].

**Important**: The CORGI component library uses submodules, so download the
source using:

~~~{.sh}
  git clone --recursive https://github.com/google/corgi.git
~~~

   * Discuss CORGI with other developers and users on the
     [CORGI Google Group][].
   * File issues on the [CORGI Issues Tracker][].
   * Post your questoins to [stackoverflow.com][] with a mention of
     **fpl corgi**.

To contribute to this project see [CONTRIBUTING][].

# Concepts

The core functionality of [CORGI][] is provided by the following classes:
   * [Component][]
   * [Entity][]
   * [Entity Manager][]

Each class is described in the folloiwing sections of the [Programmer's Guide][]:
   * [Component](@ref corgi_guide_component)
      - An object that contains the logic and data pertaining to a particular
        system in the game.
   * [Entity](@ref corgi_guide_entity)
      - The basic building block of a game, that does not do much on its own. It
        can be associated with many Components to achieve more complex behavior.
   * [Entity Manager](@ref corgi_guide_entity_manager)
      - This is a the object that ties Entities and Components together. It acts
        as the main point of interface for game logic to create Entities and
        register them with Components.

# Supported Platforms

[CORGI][] has been tested on the following platforms:

   * [Android][]
   * [Linux][] (x86_64)
   * [OS X][]
   * [WIndows][]

<br>

   [Android]: http://www.android.com
   [API Reference]: @ref corgi_api_reference
   [Component]: @ref corgi_component
   [Component Library]: @ref corgi_component_library
   [CONTRIBUTING]: @ref contributing
   [CORGI Google Group]: http://groups.google.com/group/corgi-lib
   [CORGI Issues Tracker]: http://github.com/google/corgi/issues
   [CORGI]: @ref corgi_index
   [Entity]: @ref corgi_entity
   [Entity Manager]: @ref corgi_entity_manager
   [entity-component system]: https://en.wikipedia.org/wiki/Entity_component_system
   [GitHub]: http://github.com/google/corgi
   [Linux]: http://en.m.wikipedia.org/wiki/Linux
   [OS X]: http://www.apple.com/osx/
   [Programmer's Guide]: usergroup0.html
   [releases page]: http://github.com/google/corgi/releases
   [stackoverflow.com]: http://stackoverflow.com/search?q=fpl+corgi
   [Windows]: http://windows.microsoft.com/
