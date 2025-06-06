#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

using SoaVectorSizeType = uint32_t;

namespace soa {

// Specialized vector for SOA structs.
template <typename T> class SoaVector {
private:
	SoaVectorSizeType count = 0;
	T *data = nullptr;

	T *align_ptr(void *p_data, SoaVectorSizeType p_size, uint64_t p_memory_offset) {
		// Has to be aligned to avoid UB: https://lesleylai.info/en/std-align/
		void *offset_data = static_cast<std::byte *>(p_data) + p_memory_offset;
		size_t space = p_size * sizeof(T);
		void *aligned_pointer = std::align(alignof(T), sizeof(T), offset_data, space);
		return reinterpret_cast<T *>(aligned_pointer);
	}

public:
	// Do not use this directly, it has to be public. Use push_X in the SOA struct instead.
	// Invalidates pointers if additional memory is needed.
	SoaVectorSizeType push_soa_member(const T &p_elem) {
		if constexpr (!std::is_trivially_constructible_v<T>)
			new (&data[count++]) T(p_elem);
		else
			data[count++] = std::move(p_elem);

		return count;
	}

	// This can't do a normal realloc, it has to either memcpy or move the bytes otherwise the offsets break.
	void soa_realloc(void *new_data, uint64_t p_memory_offset, SoaVectorSizeType p_new_capacity) {
		if constexpr (std::is_trivially_copyable_v<T>) {
			data = reinterpret_cast<T *>(memcpy(static_cast<std::byte *>(new_data) + p_memory_offset, data, p_new_capacity * sizeof(T)));
		} else {
			T *new_column_data = align_ptr(new_data, p_new_capacity, p_memory_offset);
			for (SoaVectorSizeType i = 0; i < size(); i++)
				new (&new_column_data[i]) T(std::move(data[i]));
			data = new_column_data;
		}
	}

	void destroy_at(SoaVectorSizeType p_index) {
		if constexpr (!std::is_trivially_destructible_v<T>)
			data[p_index].~T();
	}

	void post_erase(SoaVectorSizeType p_index_to_erase, SoaVectorSizeType p_end_index) {
		void *destination = &data[p_index_to_erase];
		const void *source = &data[p_end_index];
		memcpy(destination, source, sizeof(T));

		// Only update count if the index is last value in this vector. Its possible that another SoaVector member has a larger size so the end index might not be this Vectors end index.
		if (count - 1 == p_end_index)
			count--;
	}

	void init(void *p_data, SoaVectorSizeType p_size, uint64_t p_memory_offset) {
		if constexpr (std::is_trivially_constructible_v<T>)
			data = reinterpret_cast<T *>(static_cast<std::byte *>(p_data) + p_memory_offset);
		else
			data = align_ptr(p_data, p_size, p_memory_offset);
	}

	void init_fixed(void *p_data, SoaVectorSizeType p_size, uint64_t p_memory_offset) {
		if constexpr (std::is_trivially_constructible_v<T>)
			data = reinterpret_cast<T *>(static_cast<std::byte *>(p_data) + p_memory_offset);
		else
			data = align_ptr(p_data, p_size, p_memory_offset);
		count = p_size; // for dynamic Vectors count updates when you push_back, init count for fixed vectors.
	}

	void *get_data() { return data; }
	T *ptr() { return data; }
	[[nodiscard]] const T *ptr() const { return data; }

	void clear() {
		SoaVectorSizeType p_size = 0;
		if (data == nullptr) {
			count = p_size;
			return;
		}

		if (p_size < count) {
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (SoaVectorSizeType i = p_size; i < count; i++)
					data[i].~T();
		}
		count = p_size;
	}

	void reset() {
		clear();
		if (data)
			data = nullptr;
	}

	[[nodiscard]] bool is_empty() const { return count == 0; }

	[[nodiscard]] SoaVectorSizeType size() const { return count; }

	const T &operator[](SoaVectorSizeType p_index) const { return data[p_index]; }
	T &operator[](SoaVectorSizeType p_index) { return data[p_index]; }

	[[nodiscard]] SoaVectorSizeType find(const T &p_val, SoaVectorSizeType p_from = 0) const {
		for (SoaVectorSizeType i = p_from; i < count; i++)
			if (data[i] == p_val)
				return i;
		return -1;
	}

	[[nodiscard]] bool has(const T &p_val) const { return find(p_val) != -1; }

	// Iterator API (satisfies std::ranges::contiguous_range constraints https://stackoverflow.com/a/75061822)
	template <bool IsConst> class Iterator {
	public:
		using difference_type = std::ptrdiff_t;
		using element_type = T;
		using pointer = std::conditional_t<IsConst, const T *, T *>;
		using reference = std::conditional_t<IsConst, const T &, T &>;
		using iterator_concept = std::contiguous_iterator_tag;

		Iterator(pointer p_ptr) : elem_ptr(p_ptr) {}
		Iterator() = default;

		reference operator*() const { return *elem_ptr; }
		pointer operator->() const { return elem_ptr; }

		Iterator &operator++() {
			++elem_ptr;
			return *this;
		}

		Iterator operator++(int) {
			Iterator temp = *this;
			++(*this);
			return temp;
		}

		Iterator &operator--() {
			--elem_ptr;
			return *this;
		}

		Iterator operator--(int) {
			Iterator temp = *this;
			--(*this);
			return temp;
		}

		// Random access operations
		Iterator operator+(const difference_type n) const { return Iterator(elem_ptr + n); }
		friend Iterator operator+(const difference_type value, const Iterator &other) { return other + value; }
		Iterator operator-(const difference_type n) const { return Iterator(elem_ptr - n); }
		difference_type operator-(const Iterator &other) const { return elem_ptr - other.elem_ptr; }

		// Compound assignment
		Iterator &operator+=(const difference_type n) {
			elem_ptr += n;
			return *this;
		}
		Iterator &operator-=(const difference_type n) {
			elem_ptr -= n;
			return *this;
		}

		// Subscript operator
		reference operator[](const difference_type n) const { return *(elem_ptr + n); }

		// Comparison operators
		bool operator==(const Iterator &other) const { return elem_ptr == other.elem_ptr; }
		bool operator!=(const Iterator &other) const { return !(*this == other); }
		bool operator<(const Iterator &other) const { return elem_ptr < other.elem_ptr; }
		bool operator>(const Iterator &other) const { return elem_ptr > other.elem_ptr; }
		bool operator<=(const Iterator &other) const { return !(*this > other); }
		bool operator>=(const Iterator &other) const { return !(*this < other); }
		constexpr auto operator<=>(const Iterator &rhs) const = default;

	private:
		pointer elem_ptr = nullptr;
	};

	Iterator<false> begin() { return Iterator<false>(data); }
	Iterator<false> end() { return Iterator<false>(data + size()); }
	[[nodiscard]] Iterator<true> begin() const { return Iterator<true>(ptr()); }
	[[nodiscard]] Iterator<true> end() const { return Iterator<true>(ptr() + size()); }
};

} // namespace soa