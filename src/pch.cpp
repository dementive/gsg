// Dummy file that helps figure out how to compile the pch
// To correctly compile the pch it needs to be compiled with the exact same command as all the other files in the module.
// Compiling this file will show (almost) the exact command needed to compile the pch file in the compile_commands.json file when searching for pch.cpp.

// Scons and godot have no pre compiled header support for clang so have to use stupid hacks to make it work
// Hacky steps to compile pch:
// 1. Run `bd.sh build_pch`
// 2. find the pch.cpp entry in the generated compile_commands.json file
// 3. Copy the "command" parameter string to get the full compile command.
// 4. Change "-o /path/to/pch.cpp" to -o /path/to/src/pch.hpp.pch at the front of the command
// 5. Change the command target the the end of the command to "/path/to/src/pch.hpp"
// 5b. If compiling as a shared library also have to add the -fPIC flag, I guess (https://gitlab.kitware.com/cmake/cmake/-/issues/20289#note_1268843).
// 6. cd to the godot directory
// 7. Run the compile command with the replaced output and input arguments to compile the pch file

#include "pch.hpp"
