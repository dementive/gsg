#include "NodeManager.hpp"

#include "gui/Hud.hpp"

using namespace CG;

Map3D *NM::map = nullptr;
Hud *NM::hud = nullptr;

void NM::init_gui() { hud = create_databind(Hud, "res://scenes/gui/hud.tscn"); }

void NM::clear_temporary_nodes() {
	map = nullptr; // The engine frees the map node automatically when changing scenes or quitting.
	hud->queue_free();
}
