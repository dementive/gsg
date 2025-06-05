#pragma once

#include "Vec.hpp"
#include "core/string/ustring.h"
#include "entt/entity/fwd.hpp"

namespace CG {

struct Name : String {};

struct AreaProvinces : TightVec<entt::entity> {};

struct Capital {
	entt::entity entity;

	Capital() = default;
	Capital(const entt ::entity &p_entity) : entity(p_entity) {}
	operator entt ::entity &() { return entity; }
	operator const entt ::entity &() const { return entity; }
};

struct AreaComponent {
	entt::entity entity;

	AreaComponent() = default;
	AreaComponent(const entt ::entity &p_entity) : entity(p_entity) {}
	operator entt ::entity &() { return entity; }
	operator const entt ::entity &() const { return entity; }
};

}
