#!/usr/bin/env python

import os
import subprocess
import argparse

scripts_dir = os.path.join('..', 'gsg', 'build')
debug_options = "debug_symbols=yes debug_paths_relative=yes"  # setting these in the profile doens't work for some reason
llvm_so = "bin/libgsg.linuxbsd.editor.x86_64.llvm.so"
gcc_so = "bin/libgsg.linuxbsd.editor.x86_64.so"

def clean_build(build_cmd, so_path):
	# The big up side of building as a shared library is incremental builds are fast
	# The big down side of building the module as a shared library is you get linker errors when adding any new includes from the engine that haven't been used in the module before.
	# These errors also only happen when actually running the newly unlinked code, so can show up at weird times which sucks.
	# To fix this have to delete the existing binaries and rebuild them, this takes about 5 extra seconds than only targeting the .so though.

	subprocess.run('rm -f ./bin/*', shell=True)
	result = subprocess.run(f"{build_cmd} {so_path}", shell=True)
	if result.returncode == 0:  # only build the engine if there are no compiler errors so the errors don't show twice.
		subprocess.run(f"{build_cmd}", shell=True)

def get_pch_build_command(file_path: str, json_file_path: str="compile_commands.json") -> str:
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
		with open(json_file_path, 'r') as file:
			data = json.load(file)

		if isinstance(data, list):
			for entry in data:
				if isinstance(entry, dict) and file_path in entry.get("file"):
					command = entry.get("command")
					return command
		
		print(f"No pch command found for file: {file_path}")
		return ""

	except FileNotFoundError:
		print(f"Error: The file '{json_file_path}' was not found.")
	except json.JSONDecodeError:
		print("Error: Failed to decode JSON. Please check the file format.")
	except Exception as e:
		print(f"An unexpected error occurred: {e}")

def build_pch(pch_path: str):
	result = subprocess.run(f'scons build_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} compiledb=yes', shell=True)
	if result.returncode != 0:
		return

	command = get_pch_build_command(pch_path + ".cpp")
	if command:
		command = command.replace(pch_path + ".os", pch_path + ".hpp.pch")
		command = command.replace(pch_path + ".cpp", pch_path + ".hpp")

		print("Precompiling header: ", pch_path + ".hpp")
		subprocess.run(command, shell=True)

parser = argparse.ArgumentParser(description='Build script for Godot project.')
parser.add_argument('command', choices=[
	'linux_debug', 'linux_debug_gcc', 'clean', 'fix', 'linux_debug_engine',
	'linux_debug_engine_gcc', 'static', 'linux_debug_static', 'linux_release',
	'windows_debug', 'windows_release', 'compile_timing', 'build_pch', 
	'use_pch', 'use_pch_fix', 'test'], nargs='?', help='Build command to execute.')
parser.add_argument('export', nargs='?', help='Export option for the build.')

args = parser.parse_args()

# Change directory to the godot folder
os.chdir('../godot')

# Run compile command
if args.command == 'linux_debug':
	subprocess.run(f'scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}', shell=True)
elif args.command == 'linux_debug_gcc':
	subprocess.run(f'scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} use_llvm=no {gcc_so}', shell=True)
elif args.command in ['clean', 'fix', 'linux_debug_engine']:
	clean_build(f'scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options}', llvm_so)
elif args.command == 'linux_debug_engine_gcc':
	clean_build(f'scons shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} use_llvm=no', gcc_so)
elif args.command in ['static', 'linux_debug_static']:
	# Statically links the module into the godot binary
	subprocess.run(f'scons profile={scripts_dir}/linux_debug.py {debug_options}', shell=True)
elif args.command == 'linux_release':
	subprocess.run(f'scons profile={scripts_dir}/linux_release.py', shell=True)
elif args.command == 'windows_debug':
	subprocess.run(f'scons profile={scripts_dir}/windows_debug.py', shell=True)
elif args.command == 'windows_release':
	subprocess.run(f'scons profile={scripts_dir}/windows_release.py', shell=True)
elif args.command == 'compile_timing':
	subprocess.run('/home/dm/Templates/ClangBuildAnalyzer/build/ClangBuildAnalyzer --start /home/dm/dev/gsg/src/game', shell=True)
	subprocess.run(f'scons cxxflags=-ftime-trace use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}', shell=True)
	subprocess.run('/home/dm/Templates/ClangBuildAnalyzer/build/ClangBuildAnalyzer --stop /home/dm/dev/gsg/src/game /home/dm/dev/gsg/build/test_timing', shell=True)
	subprocess.run('/home/dm/Templates/ClangBuildAnalyzer/build/ClangBuildAnalyzer --analyze /home/dm/dev/gsg/build/test_timing', shell=True)
elif args.command == 'build_pch':
	build_pch("gsg/src/gsg/pch")
elif args.command == 'use_pch':
	subprocess.run(f'scons use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}', shell=True)
elif args.command == 'use_pch_fix':
	clean_build(f'scons use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options}', llvm_so)
elif args.command == 'test':
	subprocess.run(f'scons use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}', shell=True)
else:
	subprocess.run(f'scons use_pch=yes shared_library_module=yes profile={scripts_dir}/linux_debug.py {debug_options} {llvm_so}', shell=True)

# Project export
if args.command == 'linux_release' and args.export == 'export':
	export_dir = './game/bin/export/linux'
	subprocess.run(f'godot --headless --export-release "Linux" {export_dir}/Game.x86_64', shell=True)
if args.command == 'linux_debug' and args.export == 'export':
	export_dir = './game/bin/export/linux'
	subprocess.run(f'godot --headless --export-debug "Linux" {export_dir}/Game.x86_64', shell=True)
if args.command == 'windows_release' and args.export == 'export':
	export_dir = './game/bin/export/windows'
	subprocess.run(f'godot --headless --export-release "Windows" {export_dir}/Game.exe', shell=True)
if args.command == 'windows_debug' and args.export == 'export':
	export_dir = './game/bin/export/windows'
	subprocess.run(f'godot --headless --export-release "Windows" {export_dir}/Game.exe', shell=True)
