#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/core/hashed_string.hpp"

using namespace entt::literals;

// Entt config: https://github.com/skypjack/entt/wiki/Configuration
#define ENTT_NOEXCEPTION
// #define ENTT_DISABLE_ASSERT

namespace CG {

// Tags for every entity type
// By default entt doesn't have a great way to iterate over every entity of a certain type, all entities are always the same type unless they come from a different registry.
// Adding a unique tag to every entity allows getting all components with that tag to get all entites of a certain type.
using ProvinceTag = entt::tag<"Province"_hs>;
using CountryTag = entt::tag<"Country"_hs>;
using RegionTag = entt::tag<"Region"_hs>;
using AreaTag = entt::tag<"Area"_hs>;
using ProvinceAdjacencyTag = entt::tag<"ProvinceAdjacency"_hs>;
using ProvinceBorderTag = entt::tag<"ProvinceBorder"_hs>;

template<typename T>
concept IsEntityTag = std::is_same_v<T, ProvinceTag> || std::is_same_v<T, CountryTag> || std::is_same_v<T, RegionTag> || std::is_same_v<T, AreaTag> || std::is_same_v<T, ProvinceBorderTag> || std::is_same_v<T, ProvinceAdjacencyTag>;


// Weak types for each entity type.
// These don't actually do anything since all entity types are entt::entity but make it more clear which entity a piece code is supposed to operate on.
using Entity = entt::entity;
using ProvinceEntity = entt::entity;
using CountryEntity = entt::entity;
using RegionEntity = entt::entity;
using AreaEntity = entt::entity;
using ProvinceAdjacencyEntity = entt::entity;
using ProvinceBorderEntity = entt::entity;

}
