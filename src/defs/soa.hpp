#pragma once

#include "defs/ForEachMacro.hpp"
#include "templates/SoaVector.hpp"
#include "core/templates/local_vector.h"
#include "defs/types.hpp"

/* Macros to put on a struct so SOA data layout isn't awful to work with

Example:

struct Provinces {
	FixedSizeSOA(8,
		LocalVector<ProvinceType>, type,
		String, name, // loc key
		Vector2, centroid, // mean point in province
		float, orientation, // angle in radians
		Entity, owner, // id of owning country
		Entity, area, // area id
		LocalVector<Entity>, adjacencies, // ProvinceAdjacency ids
		LocalVector<Entity>, borders // ProvinceBorder ids
	)

	void initialize(Entity p_size) {
		init(p_size);

		Entity *ptr = owner.ptr();
		for (Entity i = 0; i < p_size; ++i) {
			ptr[i] = ENTITY_MAX;
		}
	}
};

struct test {
	Provinces x;

	void test_thing() {
		x.initialize(100);
	}
};
*/

#define SOA_VECTOR_TYPE(m_type) LocalVector<m_type, Entity>
#define SOA_FIXED_VECTOR_TYPE(m_type) SoaVector<m_type, Entity>

#define SOA_FIXED_TYPES(m_type, m_name) \
	SOA_FIXED_VECTOR_TYPE(m_type) m_name;

#define SOA_TYPES(m_type, m_name) \
	SOA_VECTOR_TYPE(m_type) m_name;

#define SOA_INIT(m_type, m_name) \
	m_name.init(data, p_size, memory_offsets[current_column]); \
	current_column++;

#define SOA_GET_MALLOC_SIZE(m_type, m_name) \
	memory_offsets[mem_offset_idx] = total_size; \
	total_size += sizeof(m_type) * p_size; \
	mem_offset_idx++;

#define SOA_SETGET(m_type, m_name) \
	void set_##m_name(Entity p_index, const m_type &p_item) { m_name[p_index] = p_item; } \
	m_type get_##m_name(Entity p_index) const { return m_name[p_index]; } \

#define SOA_RESIZE(m_type, m_name) \
	m_name.resize(p_size);

#define SOA_DESTROY(m_type, m_name) \
	m_name.reset();

#define SOA(...) \
	FOR_EACH_TWO_ARGS(SOA_TYPES, __VA_OPT__(__VA_ARGS__, )) \
	void resize(Entity p_size) { \
		FOR_EACH_TWO_ARGS(SOA_RESIZE, __VA_OPT__(__VA_ARGS__, )) \
	} \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, ))

// SOA has a problem that you end up doing more total malloc's than with AOS since each individual element gets allocated on the heap instead of whole objects.
// FixedSizeSOA attempts to manage the memory better and can be used to make it so only 1 call to malloc will be made and only when initializing the struct.
// Have to pass in the total number of SOA member variables as first arg.
#define FixedSizeSOA(m_class_name, m_total_columns, ...) \
	FOR_EACH_TWO_ARGS(SOA_FIXED_TYPES, __VA_OPT__(__VA_ARGS__, )) \
	int memory_offsets[m_total_columns]{}; \
	void *data{}; \
	constexpr int get_malloc_size(const Entity p_size) { \
		int total_size = 0; \
		int mem_offset_idx = 0; \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, )) \
		return total_size; \
	}	\
	constexpr void init(const Entity p_size) { \
		data = memalloc(get_malloc_size(p_size)); \
		int current_column = 0; \
		FOR_EACH_TWO_ARGS(SOA_INIT, __VA_OPT__(__VA_ARGS__, )) \
	} \
	~m_class_name() { \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, ))\
	} \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, ))
