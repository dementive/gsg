# gsg
Godot 4.5 Grand Strategy map prototype built as an [engine module](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/custom_modules_in_cpp.html) with C++ 23 using the [flecs](https://github.com/SanderMertens/flecs) ECS.

## Features

- 2.5D map. The scene is 3D but the map it flat.

- Province map voronoi texture loading into a lookup texture that is used for map modes and province selection.

- Generated province border meshes with different shader materials depending on the type of border.

- Map labels for provinces

- Flatmap texture that allows drawing of oceans, rivers, lakes, trees, or other map objects.

- Surround map texture that applies a cloud effect to the edge of the map.

- Map editor to edit map objects and generate map data. The map editor runs as a EditorPlugin scene in the godot editor.

- Country selection with ctrl+click

- Select provinces by clicking on them. Select all provinces a country owns by right clicking on any of their owned provinces.

- Select units by clicking on them or by dragging with a selection box.

- Grand strategy map camera with camera bounds, zooming, movement, mouse panning, rotation, and edge scrolling.

- Unit path finding between province unit locators for ships and land units.

## Compiling on linux for C++ module development

To compile you'll need to clone this repository and the godot repository and put them next to each other in a directory. Then in the godot repository switch to the `4.5` branch. There are a ton of different options to pass to scons and a lot of different ways to build, to make this easier there is a script `bd.py` that handles compiling the module.

Compiling as a shared library is faster for incremental builds, for this use the build script `bd.py linux_debug_engine`. This will compile the module as a shared library and then compile the engine and link them together. After doing this once you can use `bd.py linux_debug` to build only the shared library.

In order to run the project after compiling as a shared library you'll need to set the `LD_LIBRARY_PATH` environment variable so godot can find the shared library: `export LD_LIBRARY_PATH="path/to/godot/bin/"`.

It is also possible to statically link everything with the `bd.py linux_debug_static` command. Having to link the godot binary with the module every time adds about 8-12 seconds to incremental builds so when possible build as a shared library for faster development. Clang PCH is also setup for linux builds and is used by default, run `bd.py build_pch` to build the pch file.


## Compiling on windows

Run `bd.py windows_debug`. Shared library compilation and PCH are not setup for windows, the only option is to statically link everything.

# Images

![Map Screenshot](/assets/map.png)
