# Globals

* [Summary](#summary)
* [Objects](#objects)
* [Functions](#functions)

## Summary

In every script loaded by Critterbits, the following objects and functions are available.

## Objects

The following data are available in the global scope. These objects are not modifiable at runtime but expose additional properties and methods useful to the scripting runtime.

### input

This object contains data useful for reading the state of input devices (gamepad, keyboard, mouse). See the [section on input](input.md) for more information.

### viewport

This object represents the state of the viewport (i.e. the part of the scene currently visible to the player). See the [section on the viewport](viewport.md) for more information.

## Functions

The following functions are available in the global scope.

### close_gui(panel_id)

Closes the GUI panel with instance identifier `panel_id`.

* `(returns)`. A Boolean value, `true` if the panel was closed or `false` if no such panel was found.
* `panel_id`. The instance identifier of the panel to close, originally returned from `open_gui`.

```
var panel_id = 0;
mymodule.update = function(delta_time) {
    // toggles the GUI "menu" opened/closed when ESC is pressed
    if (input.is_key_pressed_once(input.key_codes.ESCAPE)) {
        if (panel_id == 0) {
            panel_id = open_gui("menu");
        } else {
            close_gui(panel_id);
            panel_id = 0;
        }
    }
}
```

### ease_in(start, end, percent)

Quadratically eases in (accelerates from zero) a starting point and ending point over time.

* `(returns)`. A point (x, y) coordinates.
* `start`. The starting point (x, y) coordinates.
* `end`. The ending point (x, y) coordinates.
* `percent`. A number from 0 to 1 representing the percentage of time passed.

### find_by_tag(tag, [tag, ...])

Find entities in the current scene with the given tag(s).

* `(returns)`. An array of zero or more [entity](entities.md) objects that match the list of tags.
* `tag`. One or more tags to search for.

```
mymodule.start = function() {
    // move the entities tagged with enemy1, enemy2, enemy3 down 50 pixels
    var entities = find_by_tag("enemy1", "enemy2", "enemy3");
    for (var i = 0; i < entities.length; i++) {
        entities[i].dim.y += 50;
    }
}
```

### lerp(start, end, percent)

Linearly interpolates a starting point and ending point over time.

* `(returns)`. A point (x, y) coordinates.
* `start`. The starting point (x, y) coordinates.
* `end`. The ending point (x, y) coordinates.
* `percent`. A number from 0 to 1 representing the percentage of time passed.

### open_gui(panel, [multiple])

Opens (displays) the GUI named by `panel`.

* `(returns)`. The instance identifier of the opened panel on success. On failure, returns 0.
* `panel`. The name of the GUI panel to open. This matches the name of its TOML file in the `gui` subfolder.
* `multiple`. This optional parameter, if set to `true`, allows multiple instances of `panel` to be opened at one time. Otherwise, opening the same panel again has no effect. Defaults to `false`.

```
mymodule.update = function(delta_time) {
    // opens the GUI "menu" when ESC is pressed
    if (input.is_key_pressed_once(input.key_codes.ESCAPE)) {
        open_gui("menu");
    }
}
```

### spawn(sprite, [point])

Spawns a sprite in the current scene. This creates a new instance of the named `sprite` and places it at `location`.

* `sprite`. The name of the sprite to spawn. This matches the name of its TOML file in the `sprites` subfolder.
* `point`. An optional object containing an `x` and `y` coordinate. Defaults to (0, 0).

**Note:** Sprites are not spawned immediately, which is why this function has no return value. The sprite will be queued for creation, which usually takes place at the beginning of the next update frame.

```
mymodule.start = function() {
    // spawn a sprite at (50, 120)
    spawn("my_sprite", { "x": 50, "y": 120 });
}
```

***
[[Back to index](../index.md)] [[<< Scripting](index.md)] [[Entities >>](entities.md)]
