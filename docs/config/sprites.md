# Sprites

* [Sprite Configuration](#sprite-configuration)

## Sprite Configuration

Sprites are the common entity which define interactive objects within the game engine. You will probably create more sprites than anything else when developing your game. A sprite can have a graphical representation, but this is not strictly required. Sprites can be animated, moved, scripted, and collide with other sprites.

A sprite is described by its TOML file which must be in the `sprites` subfolder. An example is shown below.

```
[sprite]
tag = "player"
script = "scripts/player.js"
tint = "#996699"
opacity = 0.5

[sprite_sheet]
image = "sheets/player.png"
tile_height = 48
tile_width = 32
tile_offset_x = 2
tile_offset_y = 4
sprite_scale = 2.0

[2d]
collision = "collide"

[[animation]]
name = "walk"
loop = true
auto_play = false
frames = [
    { prop = "frame.current", val = "0", dur = 200 },
    { prop = "frame.current", val = "1", dur = 200 },
    { prop = "frame.current", val = "2", dur = 200 }
]
```

### sprite

This section describes the general features of the sprite.

`tag`. This contains a user-defined, non-unique string identifying the sprite. This is most useful when you need to find a specific sprite or group of sprites from a script.

`script`. Contains a path to a JavaScript file that, if specified, will be executed when the sprite is loaded. The path is relative to the `sprites` subfolder.

`tint`. A color representing a tint to apply to the sprite's graphics, specified in hexadecimal format (`"#RRGGBB"`). Each pixel's color in the sprite's graphic is multiplied by this color value (source color = source color × (tint color / 255)). So, for example setting this to `"#000000"` will cause the sprite to be completely black while `"#FFFFFF"` will cause it to appear normally. Defaults to `"#FFFFFF"`.

`opacity`. A floating point number from 0.0 to 1.0 representing the sprite's opacity. This adjusts the alpha channel of the sprite's graphics.

### sprite_sheet

This section describes the graphics associated with the sprite, if any.

`image`. Contains a path to a PNG file that, if specified, will be used as a "sprite sheet" for this sprite. A sprite sheet allows you to combine multiple images into a single image by splitting it into "tiles," which are evenly-sized cells that can be referenced by their position. (Starting with 0 and increasing from left to right, top to bottom.) [See this article](https://www.codeandweb.com/what-is-a-sprite-sheet) if you need more information on sprite sheets.

`tile_height`. The height of an individual cell in the sprite's sheet, in pixels.

`tile_width`. The width of an individual cell in the sprite's sheet, in pixels.

`tile_offset_x`. This is useful if your sprite sheet is not aligned perfectly with the left edge. This value will be applied to the pixel X coordinate of tiles in the sheet. Defaults to 0.

`tile_offset_y`. This is useful if your sprite sheet is not aligned perfectly with the top edge. This value will be applied to the pixel Y coordinate of tiles in the sheet. Defaults to 0.

`sprite_scale`. A floating point number indicating a horizontal and vertical scale to apply to the sprite's graphics. Defaults to 1.0. When a sprite is first loaded, it's height and width are determined by multiplying this value by the `tile_height` and `tile_width`. If you modify any of these three values at runtime, however, you will need to adjust the sprite's width and height manually to match.

### 2d

This section affects the sprite's behavior with regards to 2D calculations made within the engine.

`collision`. This sets the sprite's collision behavior with regards to other sprites and map tiles. It has three possible values:

* `none`. This is the default and means that no collision checking is performed against this sprite.
* `collide`. Full collision detection is performed against other entities whose collision is not `none`. If it collides with another object whose collision is set to `collide`, the two will not be allowed to overlap.
* `trigger`. Collision detection is performed against other entities, however other objects will be allowed to overlap this sprite's space. The collision will only be detected when another object enters this sprite's space for the first time, and not again until it leaves that area. This is useful for triggering script behaviors when an object enter's this sprite's space.

### animation

This section can be repeated, each one a separate animation that can be played for this sprite.

`name`. This contains a user-defined string identifying the animation. This should be unique to this sprite, but does not need to be unique to other animations defined outside the current file.

`loop`. If set to `true`, this animation will repeat until explicitly stopped. Otherwise it plays once and stops.

`auto_play`. If set to `true`, this animation will start playing as soon as the sprite is loaded.

`frames`. This is an array of one or more "key frames" that describe the animation. Each key frame changes the state of the sprite by defining the following properties:

* `prop`. This is the property that should be changed, matching a supported property listed below. See the [sprite scripting guide](../scripting/sprites.md) for more information.
  * `frame.current`
  * `flip_x`
  * `flip_y`
  * more to be added ...
* `val`. The new value to set the property to. Valid values are dependent on the property being modified, though for purposes of TOML this must always be specified as a **string value**.
  * If the property is a string, this value is copied as it appears.
  * If the property is an integer, this value will be parsed into a base-10 integer.
  * If the property is a number, this value will be parsed as a base-10 floating point.
  * If the property is a Boolean, this value will be compared to the string `"true"` (case sensitive). Any other value will evaluate to `false`.
* `dur`. The number of milliseconds to wait **after** this frame is played before moving on to the next frame.

***
[[Back to index](../index.md)] [[<< Scenes](scenes.md)]