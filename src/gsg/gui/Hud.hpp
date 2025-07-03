#pragma once

#include "cg/DataBind.hpp"

namespace CG {

class Hud : public DataBind {
	GDCLASS(Hud, DataBind)

protected:
	static void _bind_methods();

public:
	Hud();
	void SetMapMode(int p_map_mode);
	String GetPlayerName();
};

} // namespace CG
