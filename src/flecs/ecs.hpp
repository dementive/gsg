#pragma once

#include "core/string/ustring.h"
#include "core/templates/fixed_vector.h"

#include "flecs.h"

using Entity = flecs::entity;
using RelationEntity = Entity;
using ScopeEntity = Entity;

class Thread;

#define inc_enum(i) ((decltype(i))(static_cast<int>(i) + 1))


enum class Relation : uint8_t {
	Capital,
	Owner,
	Unit,
	Province, // owned province
	Location,
	InArea,
	InRegion,
	ProvinceIn,
	AreaIn,
	Border,
	Adjacency, // province entity -> adjacency entity
	AdjacencyTo,
	AdjacencyFrom,
	RELATION_MAX
};

enum class Scope : uint8_t {
	Province,
	Country,
	Area,
	Region,
	Unit,
	None // Use None for entity relationships whose target has no scope.
};

// Get a relationship entity
#define Relationship(m_relationship) ecs.get_relation(Relation::m_relationship)

struct ECS : flecs::world {
	static inline ECS *self{};

	ECS();
	~ECS();

	Entity find(const String &p_name);

	Entity scope_lookup(const char *p_scope_name, const String &p_arg);

	Entity scope_lookup(Scope p_scope, const String &p_arg);

	// Check if an entity has a relationship
	bool has_relation(Entity p_entity, Relation p_relation);

	// Get the Entity that represents a Relation
	RelationEntity get_relation(Relation p_relation);

	// Get the Entity of a top level scope
	ScopeEntity get_scope(Scope p_scope);

	// Get the target of an entity's relationship
	Entity get_target(Entity p_entity, Relation p_relation);

	// Register/create relationship the entities that represent Relations
	// Note that relations must be registered after scopes because each relation assigned a scope entity.
	void register_relations();

	// Register top level scopes
	void register_scopes();

#ifdef DEBUG_ENABLED
	void run_flecs_explorer();

	Thread *explorer_thread{};
#endif

private:
	FixedVector<RelationEntity, int(Relation::RELATION_MAX)> relations;
	FixedVector<ScopeEntity, int(Scope::None)> scopes;
};
