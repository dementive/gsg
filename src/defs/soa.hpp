#pragma once

#include "ForEachMacro.hpp"
#include "templates/SoaVector.hpp"
#include "defs/types.hpp"
#include "core/variant/variant.h"

#define SOA_MAP_TYPE AHashMap<SoaVectorSizeType, SoaVectorSizeType>
#define SOA_MAP_AT_FUNC(m_entity_id) index_map[m_entity_id]
#define SOA_MAP_VALUE_NAME ->value

#define SOA_FIXED_VECTOR_TYPE(m_type) soa::SoaVector<m_type>
#define SOA_DYNAMIC_VECTOR_TYPE(m_type) soa::SoaVector<m_type>

#define SOA_FIXED_TYPES(m_type, m_name) SOA_FIXED_VECTOR_TYPE(m_type) m_name;

#define SOA_DYNAMIC_TYPES(m_type, m_name) SOA_DYNAMIC_VECTOR_TYPE(m_type) m_name;

#define SOA_INIT(m_type, m_name)                                                                                                                                                             \
	m_name.init(data, p_size, memory_offsets[current_column]);                                                                                                                               \
	current_column++;

#define SOA_INIT_FIXED(m_type, m_name)                                                                                                                                                       \
	m_name.init_fixed(data, p_size, memory_offsets[current_column]);                                                                                                                         \
	current_column++;

#define SOA_GET_MALLOC_SIZE(m_type, m_name)                                                                                                                                                  \
	memory_offsets[mem_offset_idx] = total_size;                                                                                                                                             \
	total_size += sizeof(m_type) * p_size;                                                                                                                                                   \
	mem_offset_idx++;

#define SOA_SETGET(m_type, m_name)                                                                                                                                                           \
	void set_##m_name(SoaVectorSizeType p_index, const m_type &p_item) { m_name[p_index] = p_item; }                                                                                         \
	[[nodiscard]] m_type get_##m_name(SoaVectorSizeType p_index) { return m_name[p_index]; }                                                                                                 \
	[[nodiscard]] const m_type &get_##m_name(SoaVectorSizeType p_index) const { return m_name[p_index]; }

#define SOA_PUSH(m_type, m_name)                                                                                                                                                             \
	void push_##m_name(const m_type &p_elem) {                                                                                                                                               \
		if (m_name.size() == soa_capacity) [[unlikely]] {                                                                                                                                    \
			soa_realloc();                                                                                                                                                                   \
		}                                                                                                                                                                                    \
		m_name.push_soa_member(p_elem);                                                                                                                                                      \
	}

#define SOA_MUTABLE_SETGET(m_type, m_name)                                                                                                                                                   \
	void set_##m_name(SoaVectorSizeType p_entity_id, const m_type &p_item) {                                                                                                                 \
		const SoaVectorSizeType &index = SOA_MAP_AT_FUNC(p_entity_id);                                                                                                                       \
		m_name[index] = p_item;                                                                                                                                                              \
	}                                                                                                                                                                                        \
	[[nodiscard]] m_type get_##m_name(SoaVectorSizeType p_entity_id) {                                                                                                                       \
		const SoaVectorSizeType &index = SOA_MAP_AT_FUNC(p_entity_id);                                                                                                                       \
		return m_name[index];                                                                                                                                                                \
	}                                                                                                                                                                                        \
	[[nodiscard]] const m_type &get_##m_name(SoaVectorSizeType p_entity_id) const {                                                                                                          \
		const SoaVectorSizeType &index = SOA_MAP_AT_FUNC(p_entity_id);                                                                                                                       \
		return m_name[index];                                                                                                                                                                \
	}

#define SOA_MUTABLE_PUSH(m_type, m_name)                                                                                                                                                     \
	void push_##m_name(const m_type &p_elem) {                                                                                                                                               \
		if (m_name.size() == soa_capacity) [[unlikely]] {                                                                                                                                    \
			soa_realloc();                                                                                                                                                                   \
		}                                                                                                                                                                                    \
		SoaVectorSizeType new_index = m_name.push_soa_member(p_elem) - 1;                                                                                                                    \
		soa_size = std::max(soa_size, new_index + 1);                                                                                                                                        \
		index_map[new_index] = new_index;                                                                                                                                                    \
	}

#define SOA_DEFAULT_CONSTRUCT(m_type, m_name)                                                                                                                                                \
	if constexpr (!std::is_trivially_constructible_v<m_type>) {                                                                                                                              \
		for (SoaVectorSizeType i = 0; i < p_size; ++i) {                                                                                                                                     \
			new (&m_name[i]) m_type();                                                                                                                                                       \
		}                                                                                                                                                                                    \
	}

#define SOA_REALLOC(m_type, m_name)                                                                                                                                                          \
	m_name.soa_realloc(new_data, memory_offsets[current_column], starting_capacity);                                                                                                         \
	current_column++;

#define SOA_DESTROY(m_type, m_name) m_name.reset();

#define SOA_DESTROY_AT(m_type, m_name)                                                                                                                                                       \
	if constexpr (!std::is_trivially_destructible_v<m_type>) {                                                                                                                               \
		m_name.destroy_at(index_to_erase);                                                                                                                                                   \
	}

#define SOA_POST_ERASE(m_type, m_name) m_name.post_erase(index_to_erase, end_index);

#define MutableSOA(m_class_name, m_total_columns, ...)                                                                                                                                       \
	FOR_EACH_TWO_ARGS(SOA_DYNAMIC_TYPES, __VA_OPT__(__VA_ARGS__, ))                                                                                                                          \
private:                                                                                                                                                                                     \
	void *data{};                                                                                                                                                                            \
	SoaVectorSizeType soa_capacity = 0;                                                                                                                                                      \
	SoaVectorSizeType soa_size = 0;                                                                                                                                                          \
	SOA_MAP_TYPE index_map;                                                                                                                                                                  \
	void soa_realloc() {                                                                                                                                                                     \
		const SoaVectorSizeType starting_capacity = soa_capacity;                                                                                                                            \
		soa_capacity = static_cast<SoaVectorSizeType>(soa_capacity * 1.5);                                                                                                                   \
		const SoaVectorSizeType p_size = soa_capacity;                                                                                                                                       \
                                                                                                                                                                                             \
		uint64_t total_size = 0;                                                                                                                                                             \
		int mem_offset_idx = 0;                                                                                                                                                              \
		uint64_t memory_offsets[m_total_columns];                                                                                                                                            \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, ))                                                                                                                    \
                                                                                                                                                                                             \
		void *new_data = calloc(soa_capacity, total_size / p_size);                                                                                                                          \
		int current_column = 0;                                                                                                                                                              \
		FOR_EACH_TWO_ARGS(SOA_REALLOC, __VA_OPT__(__VA_ARGS__, ))                                                                                                                            \
		free(data);                                                                                                                                                                          \
		data = new_data;                                                                                                                                                                     \
	}                                                                                                                                                                                        \
                                                                                                                                                                                             \
public:                                                                                                                                                                                      \
	void init(const SoaVectorSizeType p_size) {                                                                                                                                              \
		uint64_t total_size = 0;                                                                                                                                                             \
		int mem_offset_idx = 0;                                                                                                                                                              \
		uint64_t memory_offsets[m_total_columns];                                                                                                                                            \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, ))                                                                                                                    \
		data = calloc(p_size, total_size / p_size);                                                                                                                                          \
		soa_capacity = p_size;                                                                                                                                                               \
		int current_column = 0;                                                                                                                                                              \
		FOR_EACH_TWO_ARGS(SOA_INIT, __VA_OPT__(__VA_ARGS__, ))                                                                                                                               \
		FOR_EACH_TWO_ARGS(SOA_DEFAULT_CONSTRUCT, __VA_OPT__(__VA_ARGS__, ))                                                                                                                  \
	}                                                                                                                                                                                        \
	void erase(SoaVectorSizeType p_entity_id) {                                                                                                                                              \
		if (!index_map.has(p_entity_id)) {                                                                                                                                              \
			return;                                                                                                                                                                          \
		}                                                                                                                                                                                    \
                                                                                                                                                                                             \
		SoaVectorSizeType index_to_erase = SOA_MAP_AT_FUNC(p_entity_id);                                                                                                                     \
		const SoaVectorSizeType end_index = soa_size - 1;                                                                                                                                    \
		SoaVectorSizeType entity_id_to_move = index_map.find(end_index) SOA_MAP_VALUE_NAME;                                                                                                  \
                                                                                                                                                                                             \
		index_map.erase(p_entity_id);                                                                                                                                                        \
		index_map[entity_id_to_move] = index_to_erase;                                                                                                                                       \
		FOR_EACH_TWO_ARGS(SOA_DESTROY_AT, __VA_OPT__(__VA_ARGS__, ))                                                                                                                         \
		if (index_to_erase < --soa_size) {                                                                                                                                                   \
			FOR_EACH_TWO_ARGS(SOA_POST_ERASE, __VA_OPT__(__VA_ARGS__, ))                                                                                                                     \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	~m_class_name() {                                                                                                                                                                        \
		if (data != nullptr) {                                                                                                                                                               \
			clear();                                                                                                                                                                         \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	void clear() {                                                                                                                                                                           \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, ))                                                                                                                            \
		free(data);                                                                                                                                                                          \
	}                                                                                                                                                                                        \
	FOR_EACH_TWO_ARGS(SOA_MUTABLE_SETGET, __VA_OPT__(__VA_ARGS__, ))                                                                                                                         \
	FOR_EACH_TWO_ARGS(SOA_MUTABLE_PUSH, __VA_OPT__(__VA_ARGS__, ))

#define DynamicSOA(m_class_name, m_total_columns, ...)                                                                                                                                       \
	FOR_EACH_TWO_ARGS(SOA_DYNAMIC_TYPES, __VA_OPT__(__VA_ARGS__, ))                                                                                                                          \
private:                                                                                                                                                                                     \
	void *data{};                                                                                                                                                                            \
	SoaVectorSizeType soa_capacity = 0;                                                                                                                                                      \
	void soa_realloc() {                                                                                                                                                                     \
		const SoaVectorSizeType starting_capacity = soa_capacity;                                                                                                                            \
		soa_capacity = static_cast<SoaVectorSizeType>(soa_capacity * 1.5);                                                                                                                   \
		const SoaVectorSizeType p_size = soa_capacity;                                                                                                                                       \
                                                                                                                                                                                             \
		uint64_t total_size = 0;                                                                                                                                                             \
		int mem_offset_idx = 0;                                                                                                                                                              \
		uint64_t memory_offsets[m_total_columns];                                                                                                                                            \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, ))                                                                                                                    \
                                                                                                                                                                                             \
		void *new_data = calloc(soa_capacity, total_size / p_size);                                                                                                                          \
		int current_column = 0;                                                                                                                                                              \
		FOR_EACH_TWO_ARGS(SOA_REALLOC, __VA_OPT__(__VA_ARGS__, ))                                                                                                                            \
		free(data);                                                                                                                                                                          \
		data = new_data;                                                                                                                                                                     \
	}                                                                                                                                                                                        \
                                                                                                                                                                                             \
public:                                                                                                                                                                                      \
	void init(const SoaVectorSizeType p_size) {                                                                                                                                              \
		uint64_t total_size = 0;                                                                                                                                                             \
		int mem_offset_idx = 0;                                                                                                                                                              \
		uint64_t memory_offsets[m_total_columns];                                                                                                                                            \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, ))                                                                                                                    \
		data = calloc(p_size, total_size / p_size);                                                                                                                                          \
		soa_capacity = p_size;                                                                                                                                                               \
		int current_column = 0;                                                                                                                                                              \
		FOR_EACH_TWO_ARGS(SOA_INIT, __VA_OPT__(__VA_ARGS__, ))                                                                                                                               \
		FOR_EACH_TWO_ARGS(SOA_DEFAULT_CONSTRUCT, __VA_OPT__(__VA_ARGS__, ))                                                                                                                  \
	}                                                                                                                                                                                        \
	~m_class_name() {                                                                                                                                                                        \
		if (data != nullptr) {                                                                                                                                                               \
			clear();                                                                                                                                                                         \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	void clear() {                                                                                                                                                                           \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, ))                                                                                                                            \
		free(data);                                                                                                                                                                          \
	}                                                                                                                                                                                        \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, ))                                                                                                                                 \
	FOR_EACH_TWO_ARGS(SOA_PUSH, __VA_OPT__(__VA_ARGS__, ))

#define FixedSizeSOA(m_class_name, m_total_columns, ...)                                                                                                                                     \
	FOR_EACH_TWO_ARGS(SOA_FIXED_TYPES, __VA_OPT__(__VA_ARGS__, ))                                                                                                                            \
private:                                                                                                                                                                                     \
	void *data{};                                                                                                                                                                            \
                                                                                                                                                                                             \
public:                                                                                                                                                                                      \
	void init(const SoaVectorSizeType p_size) {                                                                                                                                              \
		uint64_t total_size = 0;                                                                                                                                                             \
		int mem_offset_idx = 0;                                                                                                                                                              \
		uint64_t memory_offsets[m_total_columns];                                                                                                                                            \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, ))                                                                                                                    \
		data = calloc(p_size, total_size / p_size);                                                                                                                                          \
		int current_column = 0;                                                                                                                                                              \
		FOR_EACH_TWO_ARGS(SOA_INIT_FIXED, __VA_OPT__(__VA_ARGS__, ))                                                                                                                         \
		FOR_EACH_TWO_ARGS(SOA_DEFAULT_CONSTRUCT, __VA_OPT__(__VA_ARGS__, ))                                                                                                                  \
	}                                                                                                                                                                                        \
	~m_class_name() {                                                                                                                                                                        \
		if (data != nullptr) {                                                                                                                                                               \
			clear();                                                                                                                                                                         \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	void clear() {                                                                                                                                                                           \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, ))                                                                                                                            \
		if (data != nullptr) {                                                                                                                                                               \
			free(data);                                                                                                                                                                      \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, ))
