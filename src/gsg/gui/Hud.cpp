#include "Hud.hpp"

#include "cg/MapMode.hpp"
#include "cg/NodeManager.hpp"

#include "Map3D.hpp"

using namespace CG;

Hud::Hud() { set_base_instance(this); }

void Hud::SetMapMode(int p_map_mode) { NM::map->set_map_mode(static_cast<MapMode>(p_map_mode)); }

void Hud::_bind_methods() { ClassDB::bind_method(D_METHOD("SetMapMode"), &Hud::SetMapMode); }
