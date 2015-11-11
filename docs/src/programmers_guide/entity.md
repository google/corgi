Entity    {#corgi_guide_entity}
======

[Entities][] are the basic building blocks of a game. By itself, an [Entity][]
does not do much. However you can register an [Entity][] with any combination
of [Component][]s to achieve complex behaviors.

The only real data that each entity contians is just some bookkeeping data
about which Components it is registered with. (Most per-Entity data is owned
by the [Component][].)

Entities are typically created by the [Entity Manager][] and referenced by
the [EntityRef][] typedef.

<br>

   [Component]: @ref corgi_guide_component
   [Entities]: @ref corgi_guide_entity
   [Entity]: @ref corgi_entity
   [EntityRef]: @ref corgi_entity_manager
   [Entity Manager]: @ref corgi_guide_entity_manager
