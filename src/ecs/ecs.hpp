#pragma once

#include "core/string/ustring.h"

#include "flecs/distr/flecs.h"

using Entity = flecs::entity;

template <typename T>
concept ScopeTypes = std::same_as<T, String> || std::same_as<T, int>;

struct ECS : flecs::world {
	static inline ECS *self{};

	ECS() {
		if (self == nullptr)
			self = this;
	}

	Entity scope_lookup(const char *p_scope_name, const ScopeTypes auto &p_arg) {
		if constexpr (std::same_as<String, decltype(p_arg)>)
			return lookup_string(p_scope_name, p_arg);
		else
			return lookup_int(p_scope_name, p_arg);
	}

	Entity scope_lookup(const Entity p_relative_entity, const ScopeTypes auto &p_arg) {
		if constexpr (std::same_as<String, decltype(p_arg)>)
			return relative_lookup_string(p_relative_entity, p_arg);
		else if constexpr (std::same_as<int, decltype(p_arg)>)
			return relative_lookup_int(p_relative_entity, p_arg);

		return entity();
	}

private:
	Entity lookup_int(const char *p_scope_name, int p_integer) {
		const String str = String(p_scope_name) + "::" + itos(p_integer);
		return lookup(str.utf8().ptr());
	}

	Entity lookup_string(const char *p_scope_name, const String &p_str) {
		const String str = String(p_scope_name) + "::" + p_str;
		return lookup(str.utf8().ptr());
	}

	Entity relative_lookup_int(const Entity p_relative_entity, int p_integer) { return p_relative_entity.lookup(itos(p_integer).utf8().ptr()); }

	Entity relative_lookup_string(const Entity p_relative_entity, const String &p_str) { return p_relative_entity.lookup(p_str.utf8().ptr()); }
};
