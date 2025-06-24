#pragma once

namespace CG {

class Map3D;
class Hud;

struct NM {
	// Game nodes, these pointers are filled when entering the main map scene.
	static Map3D *map;
	static Hud *hud;

	// Persistent nodes, these pointers are filled on startup and only destroyed when the game shuts down.
	// inline static Light3D *light = nullptr;

	// enum GuiView : uint8_t { Province, Country };

	// static void clear();
	static void init_gui();
	// static void open_gui(GuiView p_view, const Variant &p_param = Variant());

	// Remove and reset all gui views, called when entering the 'pause menu' or when opening a new panel.
	// static void close_gui(bool p_close_popups = false);
	static void clear_temporary_nodes();
};

} // namespace CG
