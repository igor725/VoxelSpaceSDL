# Voxel Space in C

## Description

Comanche's Voxel Space rendering algorithm written in C using SDL. You can try it [here](http://igvx.ru/vs_sdl/) (browser demo).

![vs_2021-08-30_12-15-23.png](https://user-images.githubusercontent.com/40758030/131331416-0491eb7e-dcde-4857-a524-c87d94ae4a76.png)


## Bindings

### Keyboard + Mouse

* G - Toggle gravitation
* LMB - Free look
* ESC - Close application
* W/S - Move camera forward/backward
* A/D - Rotate camera or strafe (if LMB pressed)
* E/Q - Adjust camera height (Jump if gravtitation enabled)
* R/F - Adjust camera pitch
* J/K - Adjust details level
* C - Reset camera pitch
* Space - Increase render distance
* Shift + Space - Decrease render distance
* LCTRL + Space - Reset render distance to default value

### Game Controller

* BACK - Toggle gravitation
* LS - Move camera
* RS - Rotate camera
* RSB - Reset camera pitch
* LT - Increase camera movement speed
* RT - Increase camera rotation speed
* LB - Decrease render distance
* RB - Increase render distance
* A - Jump (if gravitation enabled)
* B - Close application
* Y - Reset render distance to default value
* D-Pad Up/Down - Adjust camera height
* Touchpad - Look around

## Building

* On Windows you need to run the ``build.bat`` batch script in Visual Studio Developer Environment. Or... just open Visual Studio Code and press F5 button. If you need png image support, run ``build.bat sdlimage``.
* On Linux you need to run the ``build.sh`` bash script. If you need png image support, run ``build.sh sdlimage``.


## Notes

* You can change map by dropping two images (height map and diffuse map) on the Voxel Space window;
* The first letter in file name tells to the VoxelSpace how to use dropped file. ``D`` - use as a height map, and ``C`` - use as a diffuse map;
* Also you can change default map by passing two commandline arguments to VoxelSpace: ``vs.exe path/to/diffuse.png path/to/height.png``.

## Thanks

Thanks to [s-macke](https://github.com/s-macke/VoxelSpace/) for the inspiration and excellent technology explanation :)

## License

The software part of the repository is under the MIT license. Please read the license file for more information. Please keep in mind, that the Voxel Space technology might be still patented in some countries. The color and height maps are reverse engineered from the game Comanche and are therefore excluded from the license.
