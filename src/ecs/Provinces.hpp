#pragma once

#include "ecs/Registry.hpp"

namespace CG {

inline bool is_navigable_water_province(const Registry &p_registry, const Entity p_entity) { return p_registry.any_of<OceanProvinceTag, RiverProvinceTag>(p_entity); }

inline bool is_impassable_province(const Registry &p_registry, const Entity p_entity) { return p_registry.any_of<ImpassableProvinceTag, UninhabitableProvinceTag>(p_entity); }

} // namespace CG