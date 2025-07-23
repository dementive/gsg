#pragma once

#include "core/variant/variant.h"

#include "flecs/ecs.hpp"

namespace CG {

#define MAKE_SAME(m_class, m_type)                                                                                                                                                           \
	m_type same_type_value;                                                                                                                                                                  \
	m_class() = default;                                                                                                                                                                     \
	m_class(const m_type &p_##same_type_value) :                                                                                                                                             \
			same_type_value(std::move(p_##same_type_value)) {}                                                                                                                               \
	operator m_type &() { return same_type_value; }                                                                                                                                          \
	operator const m_type &() const { return same_type_value; }

#define MAKE_SAME_OTHER(m_class, m_type)                                                                                                                                                     \
	m_class(const m_type &p_##same_type_value) :                                                                                                                                             \
			same_type_value(std::move(p_##same_type_value)) {}                                                                                                                               \
	operator m_type() const { return same_type_value; }

/* Generic components */

struct LocKey {
	MAKE_SAME(LocKey, String)
	MAKE_SAME_OTHER(LocKey, StringName)
};

// Use for storing godot Node pointers in the ECS
template <typename T> struct Ptr {
	T *pointer;
	Ptr() = default;
	Ptr(T *p_ptr) :
			pointer(p_ptr) {}
	operator T *() { return pointer; }
	operator const T *() const { return pointer; }
	T *operator->() { return pointer; }
};

/* Province components */

struct CrossingLocator {
	MAKE_SAME(CrossingLocator, Vector4)
};

struct ProvinceBorderMeshRID {
	MAKE_SAME(ProvinceBorderMeshRID, RID)
};

/* Country components */

struct Player {
	MAKE_SAME(Player, Entity)
};

#undef MAKE_SAME
#undef MAKE_SAME_OTHER

} // namespace CG
