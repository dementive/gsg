#include "ecs.hpp"
#include <array>

#include "gsg/templates/ConstMap.hpp"

ECS::ECS() {
	if (self == nullptr)
		self = this;
}

Entity ECS::scope_lookup(const char *p_scope_name, const String &p_arg) {
	const String str = String(p_scope_name) + "::" + p_arg;
	return lookup(str.utf8().ptr());
}

Entity ECS::scope_lookup(const Scope p_scope, const String &p_arg) { return get_scope(p_scope).lookup(p_arg.utf8().ptr()); }

// Check if an entity has a relationship
bool ECS::has_relation(const Entity p_entity, Relation p_relation) { return p_entity.has(relations[uint8_t(p_relation)], flecs::Wildcard); }

// Get the Entity that represents a Relation
Entity ECS::get_relation(Relation p_relation) { return relations[uint8_t(p_relation)]; }

// Get the Entity of a top level scope
Entity ECS::get_scope(Scope p_scope) { return scopes[uint8_t(p_scope)]; }

// Get the target of an entity's relationship
Entity ECS::get_target(Entity p_entity, Relation p_relation) { return p_entity.target(relations[uint8_t(p_relation)]); }

// Register/create relationship the entities that represent Relations
// Note that relations must be registered after scopes because each relation assigned a scope entity.
void ECS::register_relations() {
	// clang-format off
	constexpr ConstMap<Relation, Scope, int(Relation::RELATION_MAX)> relation_scopes{
		{ Relation::Capital, Scope::Province },
		{ Relation::Owner, Scope::Country },
		{ Relation::Owns, Scope::Province },
		{ Relation::InArea, Scope::Area }, // Province in an area
		{ Relation::InRegion, Scope::Region }, // Province/area in a region
		{ Relation::ProvinceIn, Scope::Province }, // Province in a region/area
	};
	// clang-format on

	for (int i = 0; i < int(Relation::RELATION_MAX); ++i) {
		const Scope scope = relation_scopes[Relation(i)];
		const Entity scope_entity = scopes[uint8_t(scope)];
		// flecs::Relationship ensures this entity can only be used as a relationship and OneOf makes sure that it can only be used to make relationships with children of scope_entity.
		const Entity relation_entity = entity().add(flecs::Relationship).add(flecs::OneOf, scope_entity);
		relations.push_back(relation_entity);
	}
}

// Register top level scopes
void ECS::register_scopes() {
	// clang-format off
	constexpr std::array<const char *, int(Scope::SCOPE_MAX)> scope_names{
		"p",
		"c",
		"area",
		"region"
	};
	// clang-format on
	for (int i = 0; i < int(Scope::SCOPE_MAX); ++i)
		scopes.push_back(entity(scope_names[i]));
}
