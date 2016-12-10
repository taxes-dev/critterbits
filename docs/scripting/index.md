# Scripting

* [JavaScript Support](#javascript-support)
* [Script Modules](#script-modules)
* [Entity Considerations](#entity-considerations)

## JavaScript Support

Entities such as sprites can be scripted using standard JavaScript. The engine supports [ECMAScript 5.1](http://www.ecma-international.org/ecma-262/5.1/), which is beyond the scope of this document to describe, but there are plenty of resources on the internet to describe general JavaScript programming. This document will focus on the API available to games created in the Critterbits engine.

## Script Modules

Scripts are stored in discrete files in the `assets` folder. Each script uses a [module pattern](http://www.adequatelygood.com/JavaScript-Module-Pattern-In-Depth.html) to load a single object named the same as the script file itself. (For example, if the script is called `player.js`, it must return a JavaScript object called `player`.) An example of what that looks like is shown below:

```
// declare module
var player = (function() {
var player_module = {};
player_module.start = function() {
    // do initialization stuff ...
}
player_module.update = function(delta) {
    // do per-frame updates ...
}
// end module
return player_module;
}());

```

Each function to be exposed by the module must be added to the object returned. The specific signatures of functions that Critterbits understands are detailed in individual pages in this section.

It is important to keep in mind that when Critterbits loads a script, it only ever loads it _once_. That is to say, if two sprites are using the same script file, it will be loaded and parsed once and only allocated in memory once, meaning those two objects will share any objects defined in the script. For the most part this is not an issue since most of the engine interfaces call into functions that will run in the context of the object they are modifying, but it can trip you up if you define global variables in your script file that end up being modified by multiple objects.

For example, consider the following snippet:

```
var my_variable = 0;
    mymodule.update = function(delta_time) {
    if (this.dim.x < 100) {
        my_variable = this.dim.x;
    } else {
        this.dim.x = my_variable;
    }
}
```

This is actually not useful at all, but essentially what happens here is that on each update, the object is going to store the value of it's X position in the global variable `my_variable` if it is less than 100, or reset it to the value of `my_variable` if it's not.

If only one object uses this script ever, then this is fine. But if two objects are on the screen calling this script, they will constantly be overwriting each other's attempts to set the value of `my_variable` and possibly reading back the wrong value, so the behavior becomes nonsense.

> In a future update of Critterbits, entities will have a state object where they can store variables that need to persist from one call to the next in order to address this limitation.

## Entity Considerations

When Critterbits calls script functions, it sets up a context on each call that includes the entity that is currently executing the script as the `this` object. Additional methods can also bring in more entities that are currently running inside the engine. **It is very important that you do not attempt to store references to entities from one call to the next.** These objects are not actually linked directly to the objects running in the engine's memory space. Instead, they are light copies of the data that is contained in those entities. When the execution is completed for that one function call, all entities are read back in for changes and then discarded. Any entities stored in other JavaScript variables will have this link broken on the next call and will not work properly.

***
[[Back to index](../index.md)] [[Globals >>](globals.md)]