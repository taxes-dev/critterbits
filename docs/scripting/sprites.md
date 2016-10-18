# Sprites

* [Summary](#summary)
* [Events](#events)
* [Properties](#properties)
* [Methods](#methods)

## Summary

Sprites are the most common entity utilized by Critterbits, representing game objects that can display images, handle collisions, move about independently, and more. Sprites are an extension of the base [entity](entities.md) object and have all the same events, properties, and methods as well as the ones listed below.

## Events

These are the function hooks that can be declared in any sprite script. `this` is set to the current entity during these events.

### oncollision(entity)

This function is called when the current sprite collides with another entity and both have collision enabled.

* `entity`. The entity that the current sprite collided with.

**Note:** Be extremely careful when updating either entity's position from within this function. The engine already handles preventing the two from overlapping (if full collision is enabled), and it is possible you can cause an infinite collision loop if you cause the two to touch again.

Example:

```
function oncollision(other) {
    // I touched an enemy, reduce my HP
    if (other.tag == "enemy") {
        hp -= 1;
    }
}
```

## Properties

The following data are available on all sprites. Unless otherwise noted, properties can be modified at runtime.

### entity_type

The type of the current entity. Always returns `"sprite"`.

**Note:** This property is read-only.

### flip_x

A Boolean value indicating whether or not the sprite's graphics should be flipped along the X-axis.

### flip_y

A Boolean value indicating whether or not the sprite's graphics should be flipped along the Y-axis.

### frame.count

The total number of frames in the sprite's sheet. This is a function of dividing the sprite sheet's width and height by the `tile_width` and `tile_height`, accounting for `tile_offset_x` and `tile_offset_y`.

**Note:** This property is read-only.

### frame.current

The current frame in the sprite sheet the sprite is rendering. Frames are numbered starting at 0 and increase from left to right, top to bottom.

### opacity

A floating point number from 0.0 to 1.0 representing the sprite's opacity. This adjusts the alpha channel of the sprite's graphics.

### sprite_scale

A floating point number indicating a horizontal and vertical scale to apply to the sprite's graphics. Defaults to 1.0.

### tile_height

The height in pixels of individual tiles in the sprite's sheet.

**Note:** The overall height of a sprite by default is equal to `tile_height` × `sprite_scale`.

### tile_offset_x

The offset in pixels of X coordinates in the sprite's sheet.

### tile_offset_y

The offset in pixels of Y coordinates in the sprite's sheet.

### tile_width

The width in pixels of individual tiles in the sprite's sheet.

**Note:** The overall width of a sprite by default is equal to `tile_width` × `sprite_scale`.

### tint

A color representing a tint to apply to the sprite's graphics. This is an object with the following properties:

* `r`. The red color value from 0 to 255.
* `g`. The green color value from 0 to 255.
* `b`. The blue color value from 0 to 255.
* `a`. Unused. See `opacity` to change the sprite's alpha.

**Note:** Each pixel's color in the sprite's graphic is multiplied by this color value (source color = source color × (tint color / 255)). So, for example setting this to (0,0,0) will cause the sprite to be completely black while (255,255,255) will cause it to appear normally.

## Methods

The following methods are available on all sprites.

### animation.play(animation_name, [stop_others])

Plays one of the sprite's animations (see [sprites](../config/sprites.md) in the configuration documentation). If the animation is set to loop, it will continue to play until it is stopped, otherwise it plays once and then stops.

* `animation_name`. The name of the animation to play.
* `stop_others`. An optional Boolean value. If `true` then all of the sprite's other animations are stopped. Defaults to `false`.

**Note:** Calling this function with the name of an animation that is already playing has no effect. (I.e., it does not reset the animation or anything like that.)

```
function update(delta_time) {
    // play the "fire" animation if the enter key is pressed
    if (input.is_key_pressed(input.key_codes.ENTER)) {
        this.animation.play("fire");
    }
}
```

### animation.stop(animation_name)

Stops one of the sprite's animations (see [sprites](../config/sprites.md) in the configuration documentation).

* `animation_name`. The name of the animation to stop.

**Note:** Calling this function with the name of an animation that is not playing has no effect. (I.e. it is not an error.)

```
function update(delta_time) {
    // toggle the "fire" animation based on whether the enter key is pressed
    if (input.is_key_pressed(input.key_codes.ENTER)) {
        this.animation.play("fire");
    } else {
        this.animation.stop("fire");
    }
}
```

### animation.stop_all()

Stops all of the sprite's animations.

```
function update(delta_time) {
    this.animation.stop_all();
}
```

***
[[Back to index](../index.md)] [[<< Input](input.md)] [[Viewport >>](viewport.md)]