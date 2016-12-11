# Entities

* [Summary](#summary)
* [Events](#events)
* [Properties](#properties)
* [Methods](#methods)

## Summary

Entities form the basis of most scriptable objects in the engine. They form a common set of data and functionality that is used by sprites and other interactive objects.

## Events

These are the function hooks that can be declared in any entity script. `this` is set to the current entity during these events.

### start

This function is called once when the entity is first loaded into the current scene. You can use this to do any one-time setup.

Example:

```
mymodule.start = function() {
    // tell the viewport to follow this object
    viewport.follow(this);
}
```

### update

This function is called once per update frame (not to be confused with render frames, which determine the actual FPS of the game). The engine attempts to normalize update frames to 60 per second, but this is not guaranteed, so the function is passed a single parameter, `delta_time`.

* `delta_time`. A floating point number representing the fraction of a second that has passed since the previous update.

Example:

```
var VELOCITY = 100;
mymodule.update = function(delta_time) {
    // move at a steady velocity (100 pixels/sec) to the right
    this.pos.x += VELOCITY * delta_time;
}
```

## Properties

The following data are available on all entities. Unless otherwise noted, properties can be modified at runtime.

### dim

The dimensions of the current entity. This is an object containg the following properties:

* `w`. The width of the entity in pixels.
* `h`. The height of the entity in pixels.

### pos

The position of the current entity. This is an object containing the following properties:

* `x`. The X coordinate in pixels.
* `y`. The Y coordinate in pixels.

### tag

This contains a user-defined, non-unique string identifying the entity.

### time_scale

This is a floating point number indicating the scale of relative time for this entity. Defaults to 1.0. Setting this value to less than 1.0 results in the entity updating slower, and greater than 1.0 makes it update faster. This value essentially adjusts the `delta_time` reported to the `update` event for this entity only.

**Note:** An entity whose `time_scale` is set to 0 will no longer receive update events until its `time_scale` becomes non-zero.

## Methods

The following methods are available on all entities.

### cancel(callback_id)

Cancel the specified callback.

* `callback_id`. The identifier returned by a call to `delay` or `interval`.

**Note:** It is not an error to cancel a non-existent callback or one that was previously canceled. This method returns no information on whether it succeeded or not. 

```
function my_callback() {
    // do something
}

var my_callback_id = 0;
function start() {
    // call my_callback after 2.5s
    my_callback_id = this.delay(2500, my_callback);
}

function update(delta_time) {
    // I changed my mind, cancel the callback created by start()
    this.cancel(my_callback_id);
}
```

### destroy()

This marks the entity for destruction, removing it from the scene. Since the entity is not actually removed until the end of the current update frame, it will not invalidate any current script references.

```
function start() {
    // destroy me
    this.destroy();
}
```

### delay(milliseconds, callback)

Call `callback` after `milliseconds` time has elapsed, once. The engine attempts to be as precise as possible, but it is possible that the call may not happen until a few milliseconds after the specified delay.

* `(returns)`. An integer identifying the callback instance. This can be used with the `cancel` method.
* `milliseconds`. The number of milliseconds to wait before calling `callback`.
* `callback`. A JavaScript function to call after the delay has elapsed.

When `callback` is called, the context of `this` is set to the entity that initiated the delay.

```
function my_callback() {
    // move me to the right
    this.pos.x += 50;
}

function start() {
    // call my_callback after 2.5s
    this.delay(2500, my_callback);
}
```

### interval(milliseconds, callback)

Call `callback` after `milliseconds` time has elapsed, until it is canceled. The engine attempts to be as precise as possible, but it is possible that the call may not happen until a few milliseconds after the specified delay.

* `(returns)`. An integer identifying the callback instance. This can be used with the `cancel` method.
* `milliseconds`. The number of milliseconds to wait before calling `callback`.
* `callback`. A JavaScript function to call after the delay has elapsed.

When `callback` is called the context of `this` is set to the entity that initiated the interval. The function can also return a Boolean value; if it evalutes to `false`, the callback will be canceled.

**Note:** The `callback` will be called continuously until it is cancelled with the `cancel` method or the callback function returns `false`.

```
function my_callback() {
    // move me to the right until I pass the 500 pixel mark
    this.pos.x += 50;
    return this.dim.x < 500;
}

function start() {
    // call my_callback every .5s
    this.interval(500, my_callback);
}
```

***
[[Back to index](../index.md)] [[<< Globals](globals.md)] [[Input >>](input.md)]