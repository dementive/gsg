#pragma once

#include "core/templates/hashfuncs.h"
#include "core/templates/pair.h"

#include "ecs/ecs.hpp"

namespace CG {

using ProvinceEntity = Entity;
using CountryEntity = Entity;
using RegionEntity = Entity;
using AreaEntity = Entity;

struct EntityHasher {
	static uint64_t hash(const Entity p_entity) { return hash_one_uint64(uint64_t(p_entity)); }
};

template <typename F, typename S> struct EntityPairHash {
	static uint64_t hash(const Pair<F, S> &P) {
		const uint64_t h1 = EntityHasher::hash(P.first);
		const uint64_t h2 = EntityHasher::hash(P.second);
		return hash_one_uint64((h1 << 32) | h2);
	}
};

} // namespace CG
