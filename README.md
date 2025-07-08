# gsg
Godot 4.4 Grand Strategy map prototype built as a [C++ module](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/custom_modules_in_cpp.html) with [flecs](https://github.com/SanderMertens/flecs) ECS.

## Features

- 2.5D map. The scene is 3D but the map it flat.

- Province map voronoi texture loading into a lookup texture that is used for map modes and province selection.

- Generated province border meshes with different shader materials depending on the type of border.

- Map labels for provinces

- Label and Border meshes do not godot Nodes/Objects, they are normal C++ structs that hold RIDs from the RenderingServer. This avoids doing any kind of SceneTree processing for all of these meshes and saves a lot of memory. There can be tens of thousands of these on the map so using Nodes just won't work at scale.

- Flatmap texture that allows drawing of oceans, rivers, lakes, trees, or other map objects.

- Surround map texture that applies a cloud effect to the edge of the map.

- Map editor to edit map objects and generate map data. The map editor runs as a EditorPlugin scene in the godot editor.

- Map Data caching. Computing some map data like border segments and the province lookup image is slow because it requires iterating over every pixel of the province map and doing a bunch of math on it.
Thankfully this data doesn't actually have to be computed every time the game is run. We can do almost all of the expensive work in the map editor, cache the results, and then load them in at runtime instead of computing everything everytime.

- Country selection with ctrl+click

- Select provinces by clicking on them. Select all provinces a country owns by right clicking on any of their owned provinces.

- Select units by clicking on them or by dragging with a selection box.

- Grand strategy map camera with camera bounds, zooming, movement, mouse panning, rotation, and edge scrolling.

## TODO

- Province adjacency and crossings

- Unit path finding between province unit locators for ships and land units.

- Tooltip that shows up when hovering over a province with info about it.

- Dynamic map objects placed at locator positions (ports, cities, special buildings, etc...)

- Fog of war system

- Surround map clouds shader. This is sorta implemented but not working how I want, the clouds should extend to everywhere the camera can see.

- 3D map objects like trees

- Roads spline network

- Subdivide map mesh into a bunch of quads so culling will work. Will need to figure out correct number of subdivisions, generate the meshes, and then do some UV adjustments in the Map vertex shader.

- Improve border mesh generation and shader. UVs are broken right now so the shader is scuffed. Should also be able to specify a different texture for each border type. Border rounding should also be better so there are no sharp corners.

- Unsigned distance field gradient country border rendering

- Transfer province ownership to a different country without everything exploding.

- Camera zoom steps to display different map data layers.

## Compiling on linux for C++ module development

To compile you'll need to clone this repository and [my fork of the godot repository](https://github.com/dementive/godot) and put them next to each other in a directory. Then in the godot repository switch to the `gsg` branch. There are a ton of different options to pass to scons and a lot of different ways to build, to make this easier there is a script `bd.py` that handles compiling the module.

Compiling as a shared library is faster for incremental builds, for this use the build script `bd.py linux_debug_engine`. This will compile the module as a shared library and then compile the engine and link them together. After doing this once you can use `bd.py linux_debug` to build only the shared library (this only works if you don't have more new symbols to link from godot...see bd.py for more details).

In order to run the project after compiling as a shared library you'll need to set this environment variable in your .bash_profile to so godot can find the shared library: `export LD_LIBRARY_PATH="path/to/godot/bin/"`.

Using the shared library method has some problems though (see bd.py for more details) so sometimes you want to compile and link the module statically. This is possible with the `bd.py linux_debug_static` command. Having to link the godot binary with the module every time adds about 8-12 seconds to incremental builds so when possible build as a shared library for faster development.


## Compiling on windows

The build script won't work on windows, you'll have to write a different version of bd.py or just run `scons profile=../gsg/build/windows_debug.py` in the godot directory. As far as I know building as a shared library isn't possible on windows either so you'll have to always statically link the module.


# Images

![Map Screenshot](/assets/map.png)