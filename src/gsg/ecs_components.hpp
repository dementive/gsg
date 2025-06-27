#pragma once

#include "core/math/vector4.h"
#include "core/templates/rid.h"

#include "Locator.hpp"

namespace CG {

#define MAKE_SAME(m_class, m_type, m_name)                                                                                                                                                   \
	m_type m_name;                                                                                                                                                                           \
	m_class(const m_type &p_##m_name) :                                                                                                                                                      \
			m_name(std::move(p_##m_name)) {}                                                                                                                                                 \
	operator m_type &() { return m_name; }                                                                                                                                                   \
	operator const m_type &() const { return m_name; }

/* Province components */

struct CrossingLocator : Vector4 {};

struct ProvinceBorderMeshRID : RID {};
struct UnitLocator : Locator {};
struct TextLocator : Locator {};

#undef MAKE_SAME

} // namespace CG
