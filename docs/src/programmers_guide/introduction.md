Introduction    {#corgi_guide_introduction}
============

# About CORGI    {#corgi_guide_about_corgi}

[CORGI][] is a C++ [entity-component system][] library developed primarily for
games that focus on simplicity and flexibility.

It provides an [Entity Manager][] class, which manages all the [Entity][] and
[Component][] classes used by your game. [CORGI][] also provides a
[Component Library][], which contains a suite of common game-service Components
for your use.

# Prerequisites

[CORGI][] is written in C++. You are expected to be experienced in C++
programming. You should be comfortable with compiling, linking, and debugging.

# About This Guide

This guide provides an overview of the [CORGI][] library. It does *not* cover
every aspect of functionality provided by the library. The entire API is
documented by the [API Reference][]. In addition, a sample (under
`corgi/sample`) provides example usage of the library.

# Concepts

The core functionality of [CORGI][] is provided by the following classes:
   * [Component][]
   * [Entity][]
   * [Entity Manager][]

Each class is described in the folloiwing sections of the guide:
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

<br>

   [API Reference]: @ref corgi_api_reference
   [Component]: @ref corgi_component
   [Component Library]: @ref corgi_component_library
   [CORGI]: @ref corgi_index
   [Entity]: @ref corgi_entity
   [Entity Manager]: @ref corgi_entity_manager
   [entity-component system]: https://en.wikipedia.org/wiki/Entity_component_system
