cd game
case "$1" in
  editor)
	../../godot/bin/godot.linuxbsd.editor.x86_64.llvm -e ./project.godot $2 ;;
  editor_gcc)
	../../godot/bin/godot.linuxbsd.editor.x86_64 -e ./project.godot $2 ;;
  game)
	../../godot/bin/godot.linuxbsd.editor.x86_64.llvm scenes/map.tscn $2 ;;
  *)
	../../godot/bin/godot.linuxbsd.editor.x86_64.llvm -e ./project.godot $1 ;;
esac