Component    {#corgi_guide_component}
=========

[Components][] are objects that contain the logic and data pertaining to a
particular system in your game. Typically, good [Components][] are largely
self-contained.

The data that a [Component][] stores for each [Entity][] is a struct (or
sometimes a class) referred to as ComponentData.

[Components][] are registered with an [Entity Manager][], and the
[Entity Manager][] is then responsible for adding [Entities][] to
each [Component][]. Once per frame, the [Entity Manager][] will call
`EntityManager::UpdateComponents()`, which calls each Component's
`ComponentInterface::UpdateAllEntities()`.

# Defining the ComponentData

Before creating a [Component][], you need to create a struct (or class)
that contains all your per-[Entity][] data. In some cases, you may not have
any per-[Entity][] data. In that case, just make an empty struct.

~~~{.h}
   struct MyComponentData {
     int my_entity_data = 0;
   }
~~~

# Declaring the Component class

All [Components][] should implement the [ComponentInterface][] (although it
is often useful to extend from the [Component][] abstract class).

~~~{.h}
   class MyComponent : public entity::Component<MyComponentData> {
    public:
     // This function is unused in this example, but is required as part
     // of the `ComponentInterface` as a pure-virtual function.
     virtual void AddFromRawData(EntityRef&, const void*) {}

     // This function iterates through every Entity that is registered
     // with this Component.
     virtual void UpdateAllEntities(WorldTime);
   }
~~~

Inside each header file that declares a [Component][], you need to use the
`FPL_ENTITY_REGISTER_COMPONENT()` macro. This is
required in order to declare the necessary constants for lookups.

*Note: This should be declared outside of any namespaces!*

~~~{.h}
   FPL_ENTITY_REGISTER_COMPONENT(fpl::entity::example::MyComponent,
                                 fpl::entity::example::MyComponentData)
~~~

# Defining the Component class

Inside each source file that defines a [Component][], you need to use the
`FPL_ENTTIY_DEFINE_COMPONENT()` macro. This handles
defining the storage location for the [Component][] for a given type and
ComponentData type.

*Note: This should be declared at the top of the file outside of any
namespaces!*

~~~{.cpp}
   FPL_ENTITY_DEFINE_COMPONENT(fpl::entity::example::MyComponent,
                               fpl::entity::example::MyComponentData)
~~~

After calling the above macro, you can implement any necessary methods for
your [Component][] clsas. Namely, you will want to define
`ComponentInterface::UpdateAllEntities()` with the functionality to update
each Entity each frame.

~~~{.cpp}
  void MyComponent::UpdateAllEntities(WorldTime) {
    // Iterate through each of the ComponentData that correspond to each Entity.
    for (auto iter = component_data_.begin(); iter != component_data_.end();
         ++iter) {
      MyComponentData* entity_data = Data<MyComponentData>(iter->entity);

      // Do something with the data for this Entity here.
    }
  }
~~~

<br>

   [Component]: @ref corgi_component
   [ComponentInterface]: component_interface.html
   [Components]: @ref corgi_guide_component
   [Entity]: @ref corgi_guide_entity
   [Entity Manager]: @ref corgi_guide_entity_manager
   [Entities]: @ref corgi_guide_entity
