cd ../godot

scripts_dir=../gsg/build
debug_options="debug_symbols=yes debug_paths_relative=yes" # setting these in the profile doens't work for some reason
llvm_so="bin/libgame.linuxbsd.editor.x86_64.llvm.so"
gcc_so="bin/libgame.linuxbsd.editor.x86_64.so"

case "$1" in
  linux_debug)
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options $llvm_so;;
  linux_debug_gcc)
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options use_llvm=no $gcc_so;;
  clean|fix|linux_debug_engine)
    # The big up side of building as a shared library is incremental builds are crazy fast
    # The big down side of building the module as a shared library is you get linker errors when adding any new includes from the engine that haven't been used in the module before.
    # These errors also only happen when actually running the newly unlinked code, so can show up at weird times which sucks.
    # To fix this have to delete the existing binaries and rebuild them, this takes about 5-15 extra seconds than only targeting the .so though.
    rm -f ./bin/*
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options $llvm_so
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options;;
  linux_debug_engine_gcc)
    rm -f ./bin/*
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options use_llvm=no $gcc_so
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options use_llvm=no;;
  static|linux_debug_static)
    # Statically links the module into the godot binary
    scons profile=$scripts_dir/linux_debug.py; $debug_options;;
  linux_release)
    scons profile=$scripts_dir/linux_release.py ;;
  windows_debug)
    scons profile=$scripts_dir/windows_debug.py ;;
  windows_release)
    scons profile=$scripts_dir/windows_release.py ;;
  *)
    scons game_shared=yes profile=$scripts_dir/linux_debug.py $debug_options $llvm_so ;;
esac

# if [[ "$1" == "linux_release" && "$2" == "export" ]]; then
#   export_dir=./game/bin/export/linux
# 	godot --headless --export-release "Linux" $export_dir/Game.x86_64
# fi

# if [[ "$1" == "linux_debug" && "$2" == "export" ]]; then
#   export_dir=./game/bin/export/linux
# 	godot --headless --export-debug "Linux" $export_dir/Game.x86_64
# fi

# if [[ "$1" == "windows_release" && "$2" == "export" ]]; then
#   export_dir=./game/bin/export/windows
# 	godot --headless --export-release "Windows" $export_dir/Game.exe
# fi

# if [[ "$1" == "windows_debug" && "$2" == "export" ]]; then
#   export_dir=./game/bin/export/windows
# 	godot --headless --export-release "Windows" $export_dir/Game.exe
# fi

# exit 0