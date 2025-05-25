#pragma once

#include "core/error/error_macros.h"

// This is  a simpler version of LocalVector. Main differences are it never allocates, cannot resize after creation, and can only be modified by indexing.
template <typename T, typename U = uint32_t, bool force_trivial = false>
class SoaVector {
private:
	U count = 0;
	T *data = nullptr;

public:
	void init(void *p_data, U p_size, int memory_offset) {
		data = (T*)static_cast<std::byte*>(p_data) + memory_offset;
		count = p_size;
	}

	T *ptr() { return data; }

	const T *ptr() const { return data; }

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
			data = nullptr;
		}
	}

	_FORCE_INLINE_ bool is_empty() const { return count == 0; }

	_FORCE_INLINE_ U size() const { return count; }
	_FORCE_INLINE_ const T &operator[](U p_index) const {
		CRASH_BAD_UNSIGNED_INDEX(p_index, count);
		return data[p_index];
	}
	_FORCE_INLINE_ T &operator[](U p_index) {
		CRASH_BAD_UNSIGNED_INDEX(p_index, count);
		return data[p_index];
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
		Iterator() = default;
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
		ConstIterator() = default;
		ConstIterator(const ConstIterator &p_it) { elem_ptr = p_it.elem_ptr; }

	private:
		const T *elem_ptr = nullptr;
	};

	_FORCE_INLINE_ Iterator begin() { return {data}; }
	_FORCE_INLINE_ Iterator end() { return {data + size()}; }

	_FORCE_INLINE_ ConstIterator begin() const { return {ptr()}; }
	_FORCE_INLINE_ ConstIterator end() const { return {ptr() + size()}; }

	_FORCE_INLINE_ SoaVector() = default;
	_FORCE_INLINE_ ~SoaVector() {
		if (data) {
			reset();
		}
	}
};
