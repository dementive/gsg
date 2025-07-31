#!/usr/bin/env python

import os
import subprocess
import argparse

scripts_dir = os.path.join("..", "gsg", "build")
debug_options = "debug_symbols=yes debug_paths_relative=yes"  # setting these in the profile doens't work for some reason
llvm_so = "bin/libgsg.linuxbsd.editor.x86_64.llvm.so"
gcc_so = "bin/libgsg.linuxbsd.editor.x86_64.so"


def run(cmd: str):
    return subprocess.run(cmd, shell=True)


def get_pch_build_command(file_path: str, json_file_path: str = "compile_commands.json") -> str:
    """
    Extracts the "command" associated with a given "file" from compile_commands.json.

    Scons and godot have no pre compiled header support for clang so have to use stupid hacks to make it work
    Hacky steps to compile pch:
    1. Run `bd.py build_pch` this will then:
    2. find the pch.cpp (file_path) entry in the generated compile_commands.json file
    3. Copy the "command" parameter string to get the full compile command.
    4. Change "-o /path/to/pch.cpp" to -o /path/to/src/pch.hpp.pch at the front of the command
    5. Change the command target the the end of the command to "/path/to/src/pch.hpp"
    6. Run the compile command with the replaced output and input arguments to compile the pch file
    """
    import json

    try:
        with open(json_file_path, "r") as file:
            data = json.load(file)

        if isinstance(data, list):
            for entry in data:
                if not isinstance(entry, dict):
                    continue

                command = entry.get("command")
                file = entry.get("file")
                if command and file and file_path in file:
                    return command

        print(f"No pch command found for file: {file_path}")
        return ""

    except FileNotFoundError:
        print(f"Error: The file '{json_file_path}' was not found.")
    except json.JSONDecodeError:
        print("Error: Failed to decode JSON. Please check the file format.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

    return ""


def build_pch(pch_path: str):
    command = get_pch_build_command(pch_path + ".cpp")
    if command:
        command = command.replace(pch_path + ".os", pch_path + ".hpp.pch")
        command = command.replace(pch_path + ".cpp", pch_path + ".hpp")
        command = command.replace("-fno-exceptions", "-fno-exceptions -fpch-codegen -fpch-preprocess -fpch-instantiate-templates")
        command = command.replace("-include-pch /home/dm/dev/gsg/src/gsg/pch.hpp.pch", "")
        command = command.replace("register_types", "pch")

        print("Precompiling header: ", pch_path.replace("register_types", "pch") + ".hpp")
        run(command)


parser = argparse.ArgumentParser(description="Build script for Godot project.")
parser.add_argument(
    "command",
    choices=[
        "linux_debug",
        "linux_debug_engine",
        "engine",
        "linux_debug_gcc",
        "linux_debug_engine_gcc",
        "engine_gcc",
        "static",
        "linux_debug_static",
        "static_gcc",
        "linux_release",
        "linux_release_debug",
        "linux_release_template",
        "linux_debug_template",
        "compile_timing",
        "build_pch",
        "use_pch",
        "windows_debug",
        "windows_release",
        "sans",
        "compiledb",
        "test"
    ],
    nargs="?",
    help="Build command to execute.",
)
parser.add_argument("export", nargs="?", help="Export option for the build.")

args = parser.parse_args()

# Change directory to the godot folder
os.chdir("../godot")

# Default with no args is to compile as a shared lib and use pch in debug mode.
DEFAULT_COMMAND = f"scons use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}"

# Run compile command
if args.command == "linux_debug":
    run(f"scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}")
elif args.command == "linux_debug_gcc":
    run(f"scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} use_llvm=no {gcc_so}")
elif args.command in ["engine", "linux_debug_engine"]:
    # Builds the engine with llvm
    run(f"scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} compiledb=yes")
elif args.command in ["engine_gcc", "linux_debug_engine_gcc"]:
    # Builds the engine with gcc
    run(f"scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} use_llvm=no")
elif args.command in ["static", "linux_debug_static"]:
    # Statically links the module into the godot binary
    run(f"scons profile={scripts_dir}/linux_debug.py {debug_options}")
elif args.command in ["static_gcc"]:
    run(f"scons profile={scripts_dir}/linux_debug.py {debug_options} use_llvm=no")
elif args.command in ["sans"]:
    # static llvm debug build with sanitizers
    run(f"scons profile={scripts_dir}/linux_debug.py {debug_options} scu_build=no use_ubsan=yes")
elif args.command == "linux_release":
    run(f"scons profile={scripts_dir}/linux_release.py")
elif args.command == "linux_release_template":
    run(f"scons profile={scripts_dir}/linux_release.py target=template_release")
elif args.command == "linux_debug_template":
    run(f"scons profile={scripts_dir}/linux_release.py {debug_options} target=template_debug")
elif args.command == "linux_release_debug":
    run(f"scons profile={scripts_dir}/linux_release.py {debug_options}")
elif args.command == "windows_debug":
    run(f"scons profile={scripts_dir}/windows_debug.py")
elif args.command == "windows_release":
    run(f"scons profile={scripts_dir}/windows_release.py")
elif args.command == "compile_timing":
    run("/home/dm/Documents/ClangBuildAnalyzer/build/ClangBuildAnalyzer --start /home/dm/dev/gsg/src/gsg")
    run(f"scons ccflags=-ftime-trace use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}")
    run("/home/dm/Documents/ClangBuildAnalyzer/build/ClangBuildAnalyzer --stop /home/dm/dev/gsg/src/gsg /home/dm/dev/gsg/build/test_timing")
    run("/home/dm/Documents/ClangBuildAnalyzer/build/ClangBuildAnalyzer --analyze /home/dm/dev/gsg/build/test_timing")
elif args.command == "test":
    DEFAULT_COMMAND = DEFAULT_COMMAND.replace("scons", "scons sft_tests=yes").replace(llvm_so, "");
    run(DEFAULT_COMMAND)
elif args.command == "build_pch":
    build_pch("gsg/src/gsg/register_types")
elif args.command == "use_pch":
    run(DEFAULT_COMMAND)
elif args.command == "compiledb":
    DEFAULT_COMMAND = DEFAULT_COMMAND.replace("scons", "scons compiledb=yes").replace(llvm_so, "");
    run(DEFAULT_COMMAND)
else:
    run(DEFAULT_COMMAND)

# Project export
if args.command == "linux_release" and args.export == "export":
    export_dir = "./game/bin/export/linux"
    run(f'godot --headless --export-release "Linux" {export_dir}/Game.x86_64')
if args.command == "linux_debug" and args.export == "export":
    export_dir = "./game/bin/export/linux"
    run(f'godot --headless --export-debug "Linux" {export_dir}/Game.x86_64')
if args.command == "windows_release" and args.export == "export":
    export_dir = "./game/bin/export/windows"
    run(f'godot --headless --export-release "Windows" {export_dir}/Game.exe')
if args.command == "windows_debug" and args.export == "export":
    export_dir = "./game/bin/export/windows"
    run(f'godot --headless --export-release "Windows" {export_dir}/Game.exe')
