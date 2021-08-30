# Voxel Space in C

## Description

Comanche's Voxel Space rendering algorithm written in C using SDL.

## Keybindings

* LMB - Free look
* W/S - Move camera forward/backward
* A/D - Rotate camera or strafe (if LMB pressed)
* E/Q - Adjust camera height
* R/F - Adjust camera pitch
* Space - Increase render distance
* Shift + Space - Decrease render distance
* LCTRL + Space - Reset render distance to default value

## Building

* On Windows you need to run the ``build.bat`` batch script in Visual Studio Developer Environment. Or... just open Visual Studio Code and press F5 button. If you need png image support, open ``build.bat`` in the text editor and set ``USE_SDL_IMAGE`` to ``1``.
* There are no build script for Linux yet, but it can be compiled without any changes in the source code. Just use ``gcc main.c -oout/vs -DUSE_SDL_IMAGE $(pkg-config --cflags --libs sdl2 SDL2_image)``.


## Notes

* You can change map by dropping two images (height map and diffuse map) on the Voxel Space window.
* Only images with the same dimensions are supported.

## Thanks

Thanks to [s-macke](https://github.com/s-macke/VoxelSpace/) for the inspiration and excellent technology explanation :)

## License

The software part of the repository is under the MIT license. Please read the license file for more information. Please keep in mind, that the Voxel Space technology might be still patented in some countries. The color and height maps are reverse engineered from the game Comanche and are therefore excluded from the license.