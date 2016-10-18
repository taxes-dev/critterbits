# Scenes

* [Scene Configuration](#scene-configuration)
* [Tilemaps](#tilemaps)

## Scene Configuration

Scenes are a mechanism for grouping maps and sprites together to make a single coherent portion of the game, typically referred to as a "screen" or "map." A single scene can have zero or one tilemap and zero or more sprites. A scene is described by its TOML file which must be in the `scenes` subfolder.

An example is shown below.

```
[scene]
persistent = false
map = "maps/test_map.tmx"
map_scale = 2.0
script = "scripts/test_map.js"

[[sprite]]
name = "player"
at = { x = 124, y = 96 }
```
### scene

This section describes the general features of the scene.

`persistent`. If set to `true`, this scene will "persist" between scene transitions. In other words, all active sprites and scripts will be held in stasis until the scene is loaded again, rather than the scene being unloaded/reloaded. This should be used with care, since the scene will continue to occupy memory when not in use.

`map`. Contains a path to a [Tiled](http://www.mapeditor.org/) map in XML format. If specified, the map will be rendered at origin (0,0). The path is relative to the `scenes` subfolder.

`map_scale`. Sets a horizontal and vertical scale to apply when rendering the tilemap. Defaults to 1.0.

`script`. Contains a path to a JavaScript file that, if specified, will be executed when the scene is loaded. This script is not attached to any particular sprite and will continue to receive events while the scene is active. The path is relative to the `scenes` subfolder.

### sprite

This section can be repeated, each one describing a sprite to load at the same time the scene is loaded. This allows you to pre-populate the scene without the need to programmatically spawn sprites at startup.

`name`. The name of the sprite, this corresponds to its TOML file in the `sprites` subfolder.

`at`. A point (x,y) which describes the position at which the sprite will be spawned.

## Tilemaps

Critterbits uses Tiled's [TMX format](http://doc.mapeditor.org/reference/tmx-map-format/) for tilemaps. The easiest way to create these maps is to use the [Tiled map editor](http://www.mapeditor.org/). Right now Critterbites only supports orthogonal maps, but may add support for additional types (such as isometric) in the future.

While Critterbits supports the full TMX format, it interprets the map in a few ways that you will need to know in order to build effective maps.

### Map Layers

By default, layers will be rendered in the same order you have set in the map editor, but they will all be rendered as "background" unless you tell Critterbits otherwise. Background means they will appear behind all sprites in the scene.

To create a "foreground" layer, add the property `foreground` to the layer as a Boolean value and set it to `true`. Any foreground layers will appear on top of sprites in the scene but below the GUI. This can allow you to create effects like roofs, tree canopies, etc. that should appear above sprites.

### Collision

By default, nothing collides with sprites. For simplicity, individual tiles and objects can't be set to collide. Instead, put any colliding objects on a separate layer (this only work with tile layers). Then add the property `collide` to the layer as a Boolean value and set it to `true`. All tiles on this layer will now be solid to sprites.

### Object Layers

Tiled object layers are handled in a special way by the Critterbits engine. First, object layers are never rendered as visible objects (unless you enable the debug flag for this, see [engine configuration](index.md#engine-configuration)). Instead, polygons drawn on the object layer can be used as trigger regions, and sprites entering these regions can fire script events. A good example would be a region where stepping on it moves the player to a new scene. You could also use this layer to place objects that act as waypoints for AI sprites to move to.

> **this feature is still in development, details forthcoming**

***
[[Back to index](../index.md)] [[<< Configuration and Convention](index.md)] [[Sprites >>](sprites.md)]