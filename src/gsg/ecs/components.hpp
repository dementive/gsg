#pragma once

#include "core/math/vector4.h"
#include "core/templates/rid.h"

#include "cg/Locator.hpp"

namespace CG {

#define MAKE_SAME(m_class, m_type)                                                                                                                                                           \
	m_type same_type_value;                                                                                                                                                                  \
	m_class() = default;                                                                                                                                                                     \
	m_class(const m_type &p_##same_type_value) :                                                                                                                                             \
			same_type_value(std::move(p_##same_type_value)) {}                                                                                                                               \
	operator m_type &() { return same_type_value; }                                                                                                                                          \
	operator const m_type &() const { return same_type_value; }

/* Generic components */

struct LocKey {
	MAKE_SAME(LocKey, String)
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

#undef MAKE_SAME

} // namespace CG
