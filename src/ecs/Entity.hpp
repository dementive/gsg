#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/core/hashed_string.hpp"

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
