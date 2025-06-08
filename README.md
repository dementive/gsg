# gsg
Godot 4.4 Grand Strategy map prototype built as a [C++ module](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/custom_modules_in_cpp.html) using C++ 23 with [entt](https://github.com/skypjack/entt) ECS.

## Features

- 2.5D map. The scene is 3D but the map it flat.

- Province map voronoi texture loading into a lookup texture that is used for map modes and province selection.

- Generated province border meshes with different shader materials depending on the type of border.

- Map labels for provinces

- Flatmap texture that allows drawing of oceans, rivers, lakes, trees, or other map objects.

## TODO

- Map labels for regions, areas, and countries

- More map modes

- Map locators

- Map editor for quick locator placement

- Province adjacency and crossings

- Military unit selection.

- Unit path finding between province unit locators for ships and land units.

- Tooltip that shows up when hovering over a province with info about it.

- Dynamic map objects placed at locator positions (ports, cities, special buildings, etc...)

- Player country selection

- Fog of war system

- Surround map clouds shader

- 3D map objects like trees

- Roads spline network

- Subdivide map mesh into a bunch of quads so culling will work. Will need to figure out correct number of subdivisions, generate the meshes, and then do some UV adjustments in the Map vertex shader.

- Use RenderingServer to create border meshes instead of using MeshInstance3D. On a real map there will be way too many of these to have each mesh be a Node.

- Make a simpler version of Label3D that doesn't have to be a Node or Object for drawing map labels. On a real map there will be way too many of these to have each label be a Node.

- Figure out how to draw curved text. I sorta got this working with a vertex shader on a TextMesh but it didn't work very well when scaling the text or when it had to change size.

- Improve border mesh generation and shader. UVs are broken right now so the shader is scuffed. Should also be able to specify a different texture for each border type. Border rounding should also be better so there are no sharp corners.

- Unsigned distance field gradient country border rendering

- Transfer province ownership to a different country without everything exploding.

- Camera zoom steps to display different map data layers.

## Compiling on linux for C++ module development

To compile you'll need to clone this repository and the godot repository and put them next to each other in a directory. There are a ton of different options to pass to scons and a lot of different ways to build, to make this easier there is a script `bd.sh` that handles compiling the module.

Compiling as a shared library is faster for incremental builds, for this use the build script `bd.sh linux_debug_engine`. This will compile the module as a shared library and then compile the engine and link them together. After doing this once you can use `bd.sh linux_debug` to build only the shared library (this only works if you don't have more new symbols to link from godot...see bd.sh for more details).

In order to run the project after compiling as a shared library you'll need to set this environment variable in your .bash_profile to so godot can find the shared library: `export LD_LIBRARY_PATH="path/to/godot/bin/"`.

Using the shared library method has some problems though (see bd.sh for more details) so sometimes you want to compile and link the module statically. This is possible with the `bd.sh linux_debug_static` command. Having to link the godot binary with the module every time adds about 8-12 seconds to incremental builds so when possible build as a shared library for faster development.


## Compiling on windows

The build script won't work on windows, you'll have to write a different version of bd.sh or just run `scons profile=../gsg/build/windows_debug.py` in the godot directory. As far as I know building as a shared library isn't possible on windows either so you'll have to always statically link the module.


# Images

![Map Screenshot](/assets/map.png)