#pragma once

#include "templates/Vec.hpp"
#include "defs/soa.hpp"

namespace CG {

inline Vec<Color*> x;

struct Region {
	SINGLETON(Region)
	FixedSizeSOA(
		Region,4,
		String, name,
		TightVec<AreaEntity>, areas,
		ProvinceEntity, capital,
		Color, color
	)
};

struct Test {
	soa ::SoaVector<String *> name;

private:
	void *data{};

public:
	void init(const SoaVectorSizeType p_size) {
		uint64_t total_size = 0;
		int mem_offset_idx = 0;
		uint64_t memory_offsets[1];
		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(String *) * p_size;
		mem_offset_idx++;
		data = calloc(p_size, total_size / p_size);
		int current_column = 0;
		name.init_fixed(data, p_size, memory_offsets[current_column]);
		current_column++;
		if constexpr (!std ::is_trivially_constructible_v<String *>)
			for (SoaVectorSizeType i = 0; i < p_size; ++i)
				new (&name[i]) String *();
	}
	~Test() {
		if (data != nullptr)
			clear();
	}
	void clear() {
		name.reset();
		if (data != nullptr)
			free(data);
	}
	template <typename T>
	void set_name(SoaVectorSizeType p_index, T *p_item) requires(std::is_pointer_v<T*>) {
		name[p_index] = p_item;
	}

	[[nodiscard]] String *get_name(SoaVectorSizeType p_index) const { return name[p_index]; }

	template <typename T>
	[[nodiscard]] const T *get_name(SoaVectorSizeType p_index) const requires(std::is_pointer_v<T*>) {
		return name[p_index];
	}
};

}