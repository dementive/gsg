#pragma once

#include "core/templates/pair.h"

#include "entt/entity/fwd.hpp"

// Entt config: https://github.com/skypjack/entt/wiki/Configuration
// #define ENTT_NOEXCEPTION
// #define ENTT_DISABLE_ASSERT

namespace CG {

// Weak types for each entity type.
// These don't actually do anything since all entity types are entt::entity but make it more clear which entity a piece code is supposed to operate on.
using Entity = entt::entity;
using ProvinceEntity = entt::entity;
using CountryEntity = entt::entity;
using RegionEntity = entt::entity;
using AreaEntity = entt::entity;
using ProvinceAdjacencyEntity = entt::entity;
using ProvinceBorderEntity = entt::entity;

struct EntityHasher {
	static uint32_t hash(const Entity p_entity) { return hash_fmix32(static_cast<uint32_t>(p_entity)); }
};

template <typename F, typename S> struct EntityPairHash {
	static uint32_t hash(const Pair<F, S> &P) {
		uint64_t h1 = EntityHasher::hash(P.first);
		uint64_t h2 = EntityHasher::hash(P.second);
		return hash_one_uint64((h1 << 32) | h2);
	}
};

} // namespace CG
