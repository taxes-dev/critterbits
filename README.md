# Critterbits

Critterbits is a simple 2D game engine written in C++ and powered by SDL2. My main goals were to make an engine that could:
* Render 2D sprites with animation and 2D tilemaps (TMX format)
* Handle basic collision and scripted interactions
* Be flexible (use runtime interpretation of data files rather than hardcoded)

The engine isn't complete in its current state, but the basics do work. It can render maps and sprites, execute scripts, and accept input. You can read more about how to use the engine [in the documentation](docs/index.md). There's also an example project in the repository.

The engine code is licensed under the [MIT license](LICENSE). Images in the example app are part of the [Winter RPG Tiles](https://www.gamedevmarket.net/asset/winter-rpg-tiles-time-fantasy-5383/) and [80+ RPG Pixel Characters](https://www.gamedevmarket.net/asset/over-80-rpg-characters-w-animations-3540/) packs. Please purchase a license if you wish to re-use them.

You can read more about the engine [at the posts on my blog](http://taxes.moe/tags/critterbits/).

## Building
I've mostly tested the engine on Ubuntu 16.04 and Windows 7. Check [the building documentation](docs/building/index.md) for more information.

## Editor

Tiled Map Editor: http://www.mapeditor.org/

## Windows 7+ dependencies

### Tools

Visual Studio 2015, cmake 3.6.2

### Libraries

External: SDL2-2.0.4-win32-x64, SDL2-devel-2.0.4-VC, SDL2\_image-2.0.1-win32-x64, SDL2\_image-devel-2.0.1-VC,
SDL2\_ttf-2.0.14-win32-x64, SDL2\_ttf-devel-2.0.14-VC, SDL2\_gfx-1.0.1 (build from source)

Included in repository: cpptoml, duktape-1.5.1, tinyxml2-2.0.4, tmxparser, zstd

## Ubuntu 16.04+ dependencies

### Tools

APT packages: build-essential, clang-format-3.8, cmake

If building 32-bit: g++-multilib, libc6-dev:i386

### Libraries

External (APT packages): libsdl2-2.0-0, libsdl2-dev, libsdl2-gfx-1.0-0, libsdl2-gfx-dev, libsdl2-image-2.0-0, libsdl2-image-dev, libsdl2-ttf-2.0-0, libsdl2-ttf-dev

Included in repository: cpptoml, duktape-1.5.1, tinyxml2-2.0.4, tmxparser, zstd