#pragma once

#include "core/error/error_macros.h"
#include "core/os/memory.h"
#include "core/templates/sort_array.h"
#include <type_traits>

// This is basically the same as the godot LocalVector except it never allocates and cannot resize after creation
template <typename T, typename U = uint32_t, bool force_trivial = false>
class SoaVector {
private:
	U count = 0;
	U capacity = 0;
	T *data = nullptr;

public:
	void init(void *p_data, int memory_offset) {
		data = static_cast<T*>(static_cast<std::byte*>(p_data) + memory_offset);
	}

	T *ptr() {
		return data;
	}

	const T *ptr() const {
		return data;
	}

	// No push_back in SoaVector, use ptr() or operator[] instead.
	// Must take a copy instead of a reference (see GH-31736).
	// _FORCE_INLINE_ void push_back(T p_elem) {
	// 	if constexpr (!std::is_trivially_constructible_v<T> && !force_trivial) {
	// 		memnew_placement(&data[count++], T(p_elem));
	// 	} else {
	// 		data[count++] = std::move(p_elem);
	// 	}
	// }

	void invert() {
		for (U i = 0; i < count / 2; i++) {
			SWAP(data[i], data[count - i - 1]);
		}
	}

	_FORCE_INLINE_ void clear() {
		U p_size = 0;
		if (p_size < count) {
			if constexpr (!std::is_trivially_destructible_v<T> && !force_trivial) {
				for (U i = p_size; i < count; i++) {
					data[i].~T();
				}
			}
			count = p_size;
		}
	}
	_FORCE_INLINE_ void reset() {
		clear();
		if (data) {
			memfree(data);
			data = nullptr;
			capacity = 0;
		}
	}
	_FORCE_INLINE_ bool is_empty() const { return count == 0; }
	_FORCE_INLINE_ U get_capacity() const { return capacity; }

	_FORCE_INLINE_ U size() const { return count; }
	_FORCE_INLINE_ const T &operator[](U p_index) const {
		CRASH_BAD_UNSIGNED_INDEX(p_index, count);
		return data[p_index];
	}
	_FORCE_INLINE_ T &operator[](U p_index) {
		CRASH_BAD_UNSIGNED_INDEX(p_index, count);
		return data[p_index];
	}

	struct Iterator {
		_FORCE_INLINE_ T &operator*() const {
			return *elem_ptr;
		}
		_FORCE_INLINE_ T *operator->() const { return elem_ptr; }
		_FORCE_INLINE_ Iterator &operator++() {
			elem_ptr++;
			return *this;
		}
		_FORCE_INLINE_ Iterator &operator--() {
			elem_ptr--;
			return *this;
		}

		_FORCE_INLINE_ bool operator==(const Iterator &b) const { return elem_ptr == b.elem_ptr; }
		_FORCE_INLINE_ bool operator!=(const Iterator &b) const { return elem_ptr != b.elem_ptr; }

		Iterator(T *p_ptr) { elem_ptr = p_ptr; }
		Iterator() {}
		Iterator(const Iterator &p_it) { elem_ptr = p_it.elem_ptr; }

	private:
		T *elem_ptr = nullptr;
	};

	struct ConstIterator {
		_FORCE_INLINE_ const T &operator*() const {
			return *elem_ptr;
		}
		_FORCE_INLINE_ const T *operator->() const { return elem_ptr; }
		_FORCE_INLINE_ ConstIterator &operator++() {
			elem_ptr++;
			return *this;
		}
		_FORCE_INLINE_ ConstIterator &operator--() {
			elem_ptr--;
			return *this;
		}

		_FORCE_INLINE_ bool operator==(const ConstIterator &b) const { return elem_ptr == b.elem_ptr; }
		_FORCE_INLINE_ bool operator!=(const ConstIterator &b) const { return elem_ptr != b.elem_ptr; }

		ConstIterator(const T *p_ptr) { elem_ptr = p_ptr; }
		ConstIterator() {}
		ConstIterator(const ConstIterator &p_it) { elem_ptr = p_it.elem_ptr; }

	private:
		const T *elem_ptr = nullptr;
	};

	_FORCE_INLINE_ Iterator begin() {
		return Iterator(data);
	}
	_FORCE_INLINE_ Iterator end() {
		return Iterator(data + size());
	}

	_FORCE_INLINE_ ConstIterator begin() const {
		return ConstIterator(ptr());
	}
	_FORCE_INLINE_ ConstIterator end() const {
		return ConstIterator(ptr() + size());
	}

	int64_t find(const T &p_val, U p_from = 0) const {
		for (U i = p_from; i < count; i++) {
			if (data[i] == p_val) {
				return int64_t(i);
			}
		}
		return -1;
	}

	bool has(const T &p_val) const {
		return find(p_val) != -1;
	}

	template <typename C>
	void sort_custom() {
		U len = count;
		if (len == 0) {
			return;
		}

		SortArray<T, C> sorter;
		sorter.sort(data, len);
	}

	void sort() {
		sort_custom<_DefaultComparator<T>>();
	}

	void ordered_insert(T p_val) {
		U i;
		for (i = 0; i < count; i++) {
			if (p_val < data[i]) {
				break;
			}
		}
		insert(i, p_val);
	}

	_FORCE_INLINE_ SoaVector() = default;
	_FORCE_INLINE_ ~SoaVector() {
		if (data) {
			reset();
		}
	}
};
