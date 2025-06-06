#pragma once

#include "core/templates/a_hash_map.h"

#include "entt/entity/registry.hpp" // Only include entt ECS, don't care about other parts.

#include "Components.hpp"
#include "Entity.hpp"
#include "singleton.hpp"
#include "Tags.hpp"
#include "Vec.hpp"

namespace CG {

// Wraps the main entt::registry.
struct Registry : entt::registry {
	SINGLETON(Registry)

	// Creates a new entity with tag T.
	template <IsEntityTag T> [[nodiscard]] Entity create_entity(const auto &p_identifier) {
		const Entity entity = create();
		emplace<T>(entity);

		// Insert into map...
		if constexpr (std::is_same_v<T, ProvinceTag>)
			provinces.insert_new(p_identifier, entity);
		else if constexpr (std::is_same_v<T, CountryTag>)
			countries.insert_new(p_identifier, entity);
		else if constexpr (std::is_same_v<T, RegionTag>)
			regions.insert_new(p_identifier, entity);
		else if constexpr (std::is_same_v<T, AreaTag>)
			areas.insert_new(p_identifier, entity);
		return entity;
	}

	// Get entity from identifier, note that this will crash if p_identifier is not valid. It is up to the caller to make sure the identifier is valid.
	template <IsEntityTag T> [[nodiscard]] Entity get_entity(const auto &p_identifier) const {
		if constexpr (std::is_same_v<T, ProvinceTag>)
			return provinces.get(p_identifier);
		else if constexpr (std::is_same_v<T, CountryTag>)
			return countries.get(p_identifier);
		else if constexpr (std::is_same_v<T, RegionTag>)
			return regions.get(p_identifier);
		else if constexpr (std::is_same_v<T, AreaTag>)
			return areas.get(p_identifier);
	}

	// Check if an entity tag has an identifier
	template <IsEntityTag T> [[nodiscard]] bool entity_has_identifier(const auto &p_identifier) const {
		if constexpr (std::is_same_v<T, ProvinceTag>)
			return provinces.has(p_identifier);
		else if constexpr (std::is_same_v<T, CountryTag>)
			return countries.has(p_identifier);
		else if constexpr (std::is_same_v<T, RegionTag>)
			return regions.has(p_identifier);
		else if constexpr (std::is_same_v<T, AreaTag>)
			return areas.has(p_identifier);
	}

	// Converts a PackedIntArray or PackedStringArray to an Entity vector.
	// Make sure that the entity tag T already has entities in the registry to lookup.
	template <IsEntityTag T, typename VecType = Vec<Entity>> [[nodiscard]] VecType convert_packed_array(const auto &p_array) {
		VecType entities;
		entities.resize(p_array.size());
		const auto *from_ptr = p_array.ptr();
		for (int i = 0; i < p_array.size(); ++i) {
			ERR_CONTINUE_MSG(!entity_has_identifier<T>(from_ptr[i]), vformat("%s is not a valid %s identifier.", from_ptr[i], String(typeid(T).name())));
			entities[i] = get_entity<T>(from_ptr[i]);
		}
		return entities;
	}

	// In a gsg game almost every entity has some unique behavior that has to be configured or scripted.
	// For example: Province IDS (0,1,2,3), Region Tags (france_region,italy_region,germany_region,italy_region), Country Tags (ROM, FRA, IND, BZT).
	// So every entity type needs to have a container stored outside of the ECS that maps an entity's string identifier to it's Entity.
	// To access these use get_entity.
	// Not all entities need to have an identifier map either, only one's that need to be used in configuration or script.

	AHashMap<int, Entity> provinces;
	AHashMap<StringName, Entity> countries;
	AHashMap<StringName, Entity> regions;
	AHashMap<StringName, Entity> areas;
};

} // namespace CG