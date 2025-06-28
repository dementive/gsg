#pragma once

#include <array>

// Compile time map

template <typename Key, typename Value, std::size_t Size> struct ConstMap {
	std::array<std::pair<Key, Value>, Size> data;

	constexpr Value at(const Key &key) const {
		for (const auto &pair : data)
			if (pair.first == key)
				return pair.second;

		auto var = data[(18446744073709551615UL)]; // throw compiler error if key doens't exist
		return var.second;
	}

	constexpr Value operator[](const Key &key) const {
		for (const auto &pair : data)
			if (pair.first == key)
				return pair.second;

		auto var = data[(18446744073709551615UL)];
		return var.second;
	}

	constexpr ConstMap() = default;
	constexpr ConstMap(std::initializer_list<std::pair<Key, Value>> p_init) {
		for (int i = 0; const std::pair<Key, Value> &E : p_init) {
			data[i] = E;
			i++;
		}
	};
};

// static constexpr ConstMap<std::string_view, const char *, 1> test_map{
// 	{"visible", "set_visible"}
// };

// const char *var = test_map.at("visible");