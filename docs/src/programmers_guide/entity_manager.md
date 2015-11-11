Entity Manager   {#corgi_guide_entity_manager}
==============

The [Entity Manager][] is an object that coordinates the
[entity-component system][]. It is intended to act as the main
point of interface for game logic to create [Entities][] and register them
with [Components][].

At the basic level, once it has been initialized and all the [Components][] have
been registered, then the game can simply call
`EntityManager::UpdateComponents()` once per frame.

# Registering Components

You must register each [Component][] in your game with the [Entity Manager][] by
calling `EntityManager::RegisterComponent<T>()`.

*Note: The order that you register Components with the EntityManager is the
order that they will be executed in.*

~~~{.cpp}
   EntityManager entity_manager;

   MyComponent my_component;
   MyOtherComponent my_other_component;

   // The order that Components are registered is the order that they will execute in.
   entity_manager.RegisterComponent<MyComponent>(&my_component);
   entity_manager.RegisterComponent<MyOtherComponent>(&my_other_component);
~~~

# Creating Entities

You can create entities with a call to `EntityManager::AllocateNewEntity()`. It
returns a pointer to the newly allocated [Entity][] as an [EntityRef][] typedef.

~~~{.cpp}
   EntityRef my_first_entity = entity_manager.AllocateNewEntity();
   EntityRef my_second_entity = entity_manager.AllocateNewEntity();
~~~

# Add Entities to Components

You can interchangeably add [Entities][] to [Components][]. This is done
with a call to `EntityManager::AddEntityToComponent<T>()`.

~~~{.cpp}
   // Associate `my_first_entity` with `MyComponent`.
   entity_manager.AddEntityToComponent<MyComponent>(my_first_entity);

   // Associate `my_second_entity` with both `MyComponent` and `MyOtherComponent`.
   entity_manager.AddEntityToComponent<MyComponent>(my_second_entity);
   entity_manager.AddEntityToComponent<MyOtherComponent>(my_second_entity);
~~~

# Update all Components

Once you have registered your [Components][] with your [Entity Manager][] and
added your [Entities][] to your [Components][], then you just need to call
`EntityManager::UpdateComponents()` once per frame with the `delta_time` since
the last update.

~~~{.cpp}
   while(game_is_running) {
     entity_manager.UpdateComponents(delta_time);
   }
~~~

<br>

   [Component]: @ref corgi_component
   [Components]: @ref corgi_guide_component
   [Entities]: @ref corgi_guide_entity
   [Entity]: @ref corgi_entity
   [EntityRef]: @ref corgi_entity_manager
   [entity-component system]: https://en.wikipedia.org/wiki/Entity_component_system
   [Entity Manager]: @ref corgi_entity_manager
