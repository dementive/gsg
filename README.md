# gsg
Godot 4.4 Grand Strategy map prototype built as a [C++ module](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/custom_modules_in_cpp.html) using C++ 23 with [entt](https://github.com/skypjack/entt) ECS.

## Features

- 2.5D map. The scene is 3D but the map it flat.

- Province map voronoi texture loading into a lookup texture that is used for map modes and province selection.

- Generated province border meshes with different shader materials depending on the type of border.

- Map labels for provinces, regions, areas, and countries

- Flatmap texture that allows drawing of oceans, rivers, lakes, trees, or other map objects map.

## Compiling on linux for C++ module development

To compile you'll need to clone this repository and the godot repository and put them next to each other in a directory. There are a ton of different options to pass to scons and a lot of different ways to build, to make this easier there is a script `bd.sh` that handles compiling the module.

Compiling as a shared library is faster for incremental builds, for this use the build script `bd.sh linux_debug_engine`. This will compile the module as a shared library and then compile the engine and link them together. After doing this once you can use `bd.sh linux_debug` to build only the shared library (this only works if you don't have more new symbols to link from godot...see bd.sh for more details).

In order to run the project after compiling as a shared library you'll need to set this environment variable in your .bash_profile to so godot can find the shared library: `export LD_LIBRARY_PATH="path/to/godot/bin/"`.

Using the shared library method has some problems though (see bd.sh for more details) so sometimes you want to compile and link the module statically. This is possible with the `bd.sh linux_debug_static` command. Having to link the godot binary with the module every time adds about 8-12 seconds to incremental builds so when possible build as a shared library for faster development.


## Compiling on windows

The build script won't work on windows, you'll have to write a different version of bd.sh or just run `scons profile=../gsg/build/windows_debug.py` in the godot directory. As far as I know building as a shared library isn't possible on windows either so you'll have to always statically link the module.
