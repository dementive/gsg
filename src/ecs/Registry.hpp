#pragma once

#include "Entity.hpp"

#include <entt/entity/registry.hpp> // Only include entt ECS, don't care about other parts.
#include "Vec.hpp"
#include "core/templates/a_hash_map.h"
#include "singleton.hpp"

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