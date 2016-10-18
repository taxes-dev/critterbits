# Configuration and Convention

* [Assets](#assets)
* [File Formats](#file-formats)
* [Compressed Asset Archive](#compressed-asset-archive)
* [Engine Configuration](#engine-configuration)
* [Startup Scene](#startup-scene)

## Assets

All assets to be referenced by the engine must be in a folder structure. By default, this is called `assets`. A typical asset folder/file structure looks like this:

```
assets/
    gui/
    scenes/
        startup.toml
    sprites/
    cbconfig.toml
```

At minimum, there must be two files: `cbconfig.toml` and `startup.toml`. `cbconfig.toml` is the global configuration file that tells Critterbits how to run. `startup.toml` is the first scene that will be loaded when the game starts up and must be located in the `scenes` folder. The `sprites` folder isn't strictly required, but this is where any sprites that will be loaded must be located. Likewise, the `gui` folder is where UI definitions go, though none are strictly required.

When the Critterbits executable first starts, it attempts to find the `assets` folder in the same location as the executable. Alternatively it will attempt to load a compressed version called `assets.pak` (see below). You can also override the location of either the folder or the compressed archive by passing the desired location as a command line argument:

```
critterbits ../my-assets
```

The above line would load the game's assets from a folder called `my-assets` in the directory one level up from where the executable is located. It is important to note that the path given is _relative to the executable_, not the working directory when you start the executable.

The executable does not need to be named `critterbits`. Common practice when distributing your own game would be to rename the executable to one that matches your game.

## File Formats

For simplicity, Critterbits uses only a few different file formats for assets.

### JS

The scripting language used by Critterbits is JavaScript/ECMAScript. Scripting is described in detail in the [scripting section](../scripting/index.md) of this documentation.

JS files should be saved in UTF-8 encoding.

### PNG

All image files referenced are PNG unless noted otherwise. Critterbits only supports images in PNG format, and typically these should be true color with alpha (RGBA) unless you have a specific need for a different type.

Since images tend to be loaded directly into GPU textures, ideally your PNGs should have [power of 2](http://www.thealmightyguru.com/Pointless/PowersOf2.html) dimensions. Also keep in mind that GPUs have a maximum texture size that varies based on hardware, and for most modern accelerated GPUs that is usually either 8192 or 16384 pixels square. If you ensure your images are 8192 pixels in width/height or less you should be fine. While not strictly necessary, following this guideline will allow the GPU to take advantage of additional optimizations.

### TOML

All game configuration files in Critterbits are written in a general markup language called [TOML](https://github.com/toml-lang/toml). Explanation of TOML in general is outside of the scope of this document, but more information can be found at the provided link. Specific formats used by Critterbits are explained throughout this document.

TOML files should be saved in UTF-8 encoding.

### TMX

Critterbits uses the [Tiled map editor's](http://www.mapeditor.org/) TMX format for tilemaps. See the [scene configuration page](scenes.md) for more information.

The engine supports both compressed and uncompressed TMX maps.

## Compressed Asset Archive

While developing your game, it is simplest to keep all your working files on the file system for efficiency while making changes. For distribution of your final product to end users, however, Critterbits supports a compressed archive format that packs all of the contents of the `assets` folder into a single file called `assets.pak`. You can then distribute this one file alongside the executable instead of hundreds or thousands of loose files.

To do this, a tool called `assetpacker` is included. Usage:

```
assetpacker [path] [-o file] [-q] [-?] [--continue] [--no-compress]

    path           The path to the assets folder. Defaults to "./assets"
    -o file        The name/path of the asset archive to generate. Defaults to "./assets.pak"
    -q             Quiet. Suppresses banner and info messages to stdout (errors still
                   output to stderr)
    -?             Display tool help
    --continue     Attempt to continue building the archive on error, if possible
    --no-compress  Do not compress assets
```

## Engine Configuration

As mentioned above, the `cbconfig.toml` file is critical to the operation of Critterbits and sets up the initial configuration of the game engine. The following block shows possible configuration items for this file along with their default values.

```
[debug]
draw_gui_rects = false
draw_info_pane = false
draw_map_regions = false
draw_sprite_rects = false

[window]
full_screen = false
width = 1024
height = 768
title = ""
icon = ""

[rendering]
scale = 1.0

[input]
keyboard = true
controller = false
mouse = false

[[font]]
name = ""
file = ""
size = 0
```

### debug

This section provides settings for helpful debugging features.

`draw_gui_rects`. If set to `true`, this will outline the GUI's grid layout.

`draw_info_pane`. If set to `true`, a pane displaying several stats appears at the bottom of the window. It includes useful statistics such as number of entities in the scene, FPS, and memory usage.

`draw_map_regions`. If set to `true`, this outlines regions from object layers defined in Tiled maps.

`draw_sprite_rects`. If set to `true`, this will outline sprites and collision boxes.

### window

This section configures the OS window used by Critterbits.

`full_screen`. If set to `true`, the engine defaults to full screen rather than windowed. (This overrides the `width` and `height` settings to match the display resolution.)

`width`. The width of the window in pixels.

`height`. The height of the window in pixels.

`title`. The text to display in the title bar of the window.

`icon`. Path to the PNG file representing the GUI icon for the window. Relative to the location of `cbconfig.toml`.

### rendering

This section alters how the game is rendered.

`scale`. Sets a global value for horizontal and vertical scale when frames are rendered in the engine. Because this is applied after all objects have been drawn, it also affects some of the debug frames (see `debug` section above).

### input

This section configures various input methods.

`keyboard`. If set to `true`, keyboard events will be processed.

`controller`. If set to `true`, controller/gamepad events will be processed.

`mouse`. If set to `true`, mouse events will be processed.

### font

This section can be repeated, each one describing a single TrueType font to load. These are referenced by the GUI for displaying text.

`name`. A shorthand name for referring to this particular font configuration elsewhere.

`file`. Path to the TTF file for this font. Relative to the location of `cbconfig.toml`.

`size`. Font size in points, normalized to 72 DPI (so points are the same as pixels in this context).

**Note:** Each font section describes a single font/size pairing, so if you need to use the same font at different sizes you will need to create multiple font sections for it.

Critterbits should be able to utilize any TTF file that is supported by recent versions of [libfreetype](https://www.freetype.org/).

## Startup Scene

As noted earlier, there must be a minimum of one scene in the assets folder called `scenes/startup.toml`. This is the first scene that Critterbits will load when starting the game. For more information on scene configuration, see the [next section](scenes.md).

***
[[Back to index](../index.md)] [[Scenes >>](scenes.md)]