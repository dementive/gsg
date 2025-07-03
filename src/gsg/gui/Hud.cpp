#include "Hud.hpp"

#include "cg/MapMode.hpp"
#include "cg/NodeManager.hpp"

#include "ecs/ecs.hpp"

#include "components.hpp"
#include "Map3D.hpp"

using namespace CG;

Hud::Hud() { set_base_instance(this); }

void Hud::SetMapMode(int p_map_mode) { NM::map->set_map_mode(static_cast<MapMode>(p_map_mode)); }

String Hud::GetPlayerName() {
	const Entity player = ECS::self->get<Player>();
	return tr(player.get<LocKey>());
}

void Hud::_bind_methods() {
	ClassDB::bind_method(D_METHOD("SetMapMode"), &Hud::SetMapMode);
	ClassDB::bind_method(D_METHOD("GetPlayerName"), &Hud::GetPlayerName);
}
