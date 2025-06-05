#pragma once

#include "Entity.hpp"
#include "Components.hpp"

#include <entt/entity/registry.hpp> // Only include entt ECS, don't care about other parts.
#include "core/templates/a_hash_map.h"
#include "singleton.hpp"

namespace CG {

// Wraps the main entt::registry.
struct Registry : entt::registry {
	SINGLETON(Registry)

	// Creates a new entity with tag T.
	template<IsEntityTag T>
	[[nodiscard]] entt::entity create_entity(const auto &p_identifier) {
		const entt::entity entity = create();
		emplace<T>(entity);

		// Insert into map...
		if constexpr (std::is_same_v<T, ProvinceTag>) {
			provinces.insert_new(p_identifier, entity);
		} else if constexpr (std::is_same_v<T, CountryTag>) {
			countries.insert_new(p_identifier, entity);
		} else if constexpr (std::is_same_v<T, RegionTag>) {
			regions.insert_new(p_identifier, entity);
		} else if constexpr (std::is_same_v<T, AreaTag>) {
			areas.insert_new(p_identifier, entity);
		}
		return entity;
	}

	// Get entity from identifier, note that this will crash if p_identifier is not valid. It is up to the caller to make sure the identifier is valid.
	template<IsEntityTag T>
	[[nodiscard]] entt::entity get_entity(const auto &p_identifier) const {
		if constexpr (std::is_same_v<T, ProvinceTag>) {
			return provinces.get(p_identifier);
		} else if constexpr (std::is_same_v<T, CountryTag>) {
			return countries.get(p_identifier);
		} else if constexpr (std::is_same_v<T, RegionTag>) {
			return regions.get(p_identifier);
		} else if constexpr (std::is_same_v<T, AreaTag>) {
			return areas.get(p_identifier);
		}
	}

	// Entt does not have a decent way to group "entity types" together.
	// For example: Province IDS (0,1,2,3), Region Tags (france_region,italy_region,germany_region,italy_region), Country Tags (ROM, FRA, IND, BZT).
	// In a gsg game almost every entity has some unique behavior that has to be configured or scripted.
	// So every entity type needs to have a container stored outside of the ECS that maps an each entity's string identifier to it's entt::entity.
	// To access these use get_entity, these are intended for random access not iteration.
	// Not all entities need to have an identifier map either, only one's that need to be used in configuration or script via random access.

	AHashMap<int, entt::entity> provinces;
	AHashMap<StringName, entt::entity> countries;
	AHashMap<StringName, entt::entity> regions;
	AHashMap<StringName, entt::entity> areas;
};

}