#pragma once

#include <array>

#include "core/string/ustring.h"
#include "core/templates/fixed_vector.h"

#include "templates/ConstMap.hpp"

#include "flecs/distr/flecs.h"

using Entity = flecs::entity;

#define inc_enum(i) ((decltype(i))(static_cast<int>(i) + 1))

enum class Relation : uint8_t { Capital, Owner, Owns, RELATION_MAX };

enum class Scope : uint8_t { Province, Country, Area, Region, SCOPE_MAX };

// Get a relationship entity
#define Relationship(m_relationship) ecs.get_relation(Relation::m_relationship)

struct ECS : flecs::world {
	static inline ECS *self{};

	ECS() {
		if (self == nullptr)
			self = this;
	}

	Entity scope_lookup(const char *p_scope_name, const String &p_arg) {
		const String str = String(p_scope_name) + "::" + p_arg;
		return lookup(str.utf8().ptr());
	}

	Entity scope_lookup(const Scope p_scope, const String &p_arg) { return get_scope(p_scope).lookup(p_arg.utf8().ptr()); }

	// Check if an entity has a relationship
	bool has_relation(const Entity p_entity, Relation p_relation) { return p_entity.has(relations[uint8_t(p_relation)], flecs::Wildcard); }

	// Get the Entity that represents a Relation
	Entity get_relation(Relation p_relation) { return relations[uint8_t(p_relation)]; }

	// Get the Entity of a top level scope
	Entity get_scope(Scope p_scope) { return scopes[uint8_t(p_scope)]; }

	// Get the target of an entity's relationship
	Entity get_target(const Entity p_entity, Relation p_relation) { return p_entity.target(relations[uint8_t(p_relation)]); }

	// Register/create relationship the entities that represent Relations
	// Note that relations must be registered after scopes because each relation assigned a scope entity.
	void register_relations() {
		for (int i = 0; i < int(Relation::RELATION_MAX); ++i) {
			const Scope scope = relation_scopes[Relation(i)];
			const Entity scope_entity = scopes[uint8_t(scope)];
			// flecs::Relationship ensures this entity can only be used as a relationship and OneOf makes sure that it can only be used to make relationships with children of scope_entity.
			const Entity relation_entity = entity().add(flecs::Relationship).add(flecs::OneOf, scope_entity);
			relations.push_back(relation_entity);
		}
	}

	// Register top level scopes
	void register_scopes() {
		for (int i = 0; i < int(Scope::SCOPE_MAX); ++i)
			scopes.push_back(entity(scope_names[i]));
	}

private:
	FixedVector<Entity, int(Relation::RELATION_MAX)> relations;
	FixedVector<Entity, int(Scope::SCOPE_MAX)> scopes;
	static constexpr std::array<const char *, int(Scope::SCOPE_MAX)> scope_names{ "p", "c", "area", "region" };

	static constexpr ConstMap<Relation, Scope, int(Relation::RELATION_MAX)> relation_scopes{ { Relation::Capital, Scope::Province }, { Relation::Owner, Scope::Country },
		{ Relation::Owns, Scope::Province } };
};
