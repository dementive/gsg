#pragma once

#include <entt/entity/registry.hpp> // Only include entt ECS, don't care about other parts.
#include "Vec.hpp"
#include "core/templates/a_hash_map.h"
#include "singleton.hpp"
using namespace entt::literals;

// Entt config: https://github.com/skypjack/entt/wiki/Configuration
#define ENTT_NOEXCEPTION
// #define ENTT_DISABLE_ASSERT

// Tags for every entity type
// By default entt doesn't have a great way to iterate over every entity of a certain type, all entities are always the same type unless they come from a different registry.
// Adding a unique tag to every entity allows getting all components with that tag to get all entites of a certain type.
// This lets do things like any/every/ordered/random_X (where X is a province, country, area, etc...) iterators by just sorting the tag components, which gsg needs a lot of, as well as regular ECS component access.
struct EntityTag {
	using Province = entt::tag<"Province"_hs>;
	using Country = entt::tag<"Country"_hs>;
	using Region = entt::tag<"Region"_hs>;
};

template<typename T>
concept IsEntityTag = std::is_same_v<T, EntityTag::Province> || std::is_same_v<T, EntityTag::Country> || std::is_same_v<T, EntityTag::Region>;

// Weak types for each entity type.
// These don't actually do anything since all entity types are entt::entity but make it more clear which entity a piece code is supposed to operate on.
struct EntityType {
	using Province = entt::entity;
	using Country = entt::entity;
	using Region = entt::entity;
};

// Wraps the main entt::registry.
struct Registry : entt::registry {
	SINGLETON(Registry)

	// Creates a new entity with tag T.
	template<IsEntityTag T>
	[[nodiscard]] entt::entity create_entity() {
		const EntityType::Province entity = create();
		emplace<T>(entity);
		return entity;
	}

	// Entt does not have a decent way to group "entity types" together
	// For example: Province IDS (0,1,2,3), Region Tags (france_region,italy_region,germany_region,italy_region), Country Tags (ROM, FRA, IND, BZT).
	// In a gsg game almost every entity has some unique behavior that has to be configured or scripted
	// So every entity type needs to have a container stored outside of the ECS that maps an each entity's id/string identifier to it's entt::entity.

	CG::TightVec<EntityType::Province> provinces;

	AHashMap<String, EntityType::Region> regions;
	AHashMap<String, EntityType::Country> countries;
};