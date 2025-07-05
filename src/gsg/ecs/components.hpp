#pragma once

#include "core/math/vector4.h"
#include "core/templates/rid.h"

#include "cg/Locator.hpp"

namespace CG {

class MapUnit;

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

// Use for storing godot Node pointers in the ECS
#define MAKE_SAME_PTR(m_class, m_type)                                                                                                                                                       \
	m_type *pointer;                                                                                                                                                                         \
	m_class() = default;                                                                                                                                                                     \
	m_class(m_type *p_ptr) :                                                                                                                                                                 \
			pointer(p_ptr) {}                                                                                                                                                                \
	m_type *ptr() { return pointer; }                                                                                                                                                        \
	const m_type *ptr() const { return pointer; }

/* Generic components */

struct LocKey {
	MAKE_SAME(LocKey, String)
	MAKE_SAME_OTHER(LocKey, StringName)
};

/* Province components */

struct CrossingLocator {
	MAKE_SAME(CrossingLocator, Vector4)
};

struct ProvinceBorderMeshRID {
	MAKE_SAME(ProvinceBorderMeshRID, RID)
};

struct UnitLocator : Locator {};
struct TextLocator : Locator {};

/* Country components */

struct Player {
	MAKE_SAME(Player, Entity)
};

struct UnitModel {
	MAKE_SAME_PTR(UnitModel, MapUnit)
};

#undef MAKE_SAME
#undef MAKE_SAME_OTHER
#undef MAKE_SAME_PTR

} // namespace CG
