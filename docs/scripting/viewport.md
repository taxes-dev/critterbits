# Viewport

* [Summary](#summary)
* [Properties](#properties)
* [Methods](#methods)

## Summary

The viewport represents the visible area of the current scene that the player can see in the game window. While technically it is an [entity](entities.md) in programming terms, the engine handles it as a special case.

The viewport is accessible through script via the `viewport` [global variable](globals.md).

## Properties

The following data are available on the viewport object.

### dim

The dimensions/position of the viewport. This is an object containg the following properties:

* `x`. The X coordinate in pixels.
* `y`. The Y coordinate in pixels.
* `w`. The width of the viewport in pixels.
* `h`. The height of the viewport in pixels.

**Note:** Unlike other entities, this property is managed by the engine and cannot be modified directly at runtime.

## Methods

The following methods are available on the viewport object.

### follow(entity)

Tell the viewport to automatically follow the given `entity`. The viewport's center will remain in the center of the specified entity. This is most useful for having the viewport follow a player-controlled object, but any entity is a possible target.

* `entity`. The entity to follow.

```
function start() {
    // tell the viewport to follow the current entity
    viewport.follow(this);
}
```

***
[[Back to index](../index.md)] [[<< Sprites](sprites.md)]