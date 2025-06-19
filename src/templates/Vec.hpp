#pragma once

#include <initializer_list>
#include <type_traits>

#include "core/error/error_macros.h"
#include "core/os/memory.h"
#include "core/templates/sort_array.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"

namespace CG {

#define MAKE_VARIANT_CONSTRUCTOR(m_variant_vec_type, m_variant_type, m_concept)                                                                                                              \
	_FORCE_INLINE_ Vec(const m_variant_vec_type &p_from) requires(m_concept) {                                                                                                               \
		resize(p_from.size());                                                                                                                                                               \
		const m_variant_type *from_ptr = p_from.ptr();                                                                                                                                       \
		for (U i = 0; i < p_from.size(); i++) {                                                                                                                                              \
			data[i] = from_ptr[i];                                                                                                                                                           \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	Vec &operator=(const m_variant_vec_type &p_from) requires(m_concept) {                                                                                                                   \
		resize(p_from.size());                                                                                                                                                               \
		const m_variant_type *from_ptr = p_from.ptr();                                                                                                                                       \
		for (U i = 0; i < p_from.size(); i++) {                                                                                                                                              \
			data[i] = from_ptr[i];                                                                                                                                                           \
		}                                                                                                                                                                                    \
		return *this;                                                                                                                                                                        \
	}

#define MAKE_VARIANT_CONSTRUCTOR_SAME(m_variant_vec_type, m_variant_type)                                                                                                                    \
	_FORCE_INLINE_ Vec(const m_variant_vec_type &p_from) requires(std::is_same_v<T, m_variant_type>) {                                                                                       \
		resize(p_from.size());                                                                                                                                                               \
		const m_variant_type *from_ptr = p_from.ptr();                                                                                                                                       \
		for (U i = 0; i < p_from.size(); i++) {                                                                                                                                              \
			data[i] = from_ptr[i];                                                                                                                                                           \
		}                                                                                                                                                                                    \
	}                                                                                                                                                                                        \
	Vec &operator=(const m_variant_vec_type &p_from) requires(std::is_same_v<T, m_variant_type>) {                                                                                           \
		resize(p_from.size());                                                                                                                                                               \
		const m_variant_type *from_ptr = p_from.ptr();                                                                                                                                       \
		for (U i = 0; i < p_from.size(); i++) {                                                                                                                                              \
			data[i] = from_ptr[i];                                                                                                                                                           \
		}                                                                                                                                                                                    \
		return *this;                                                                                                                                                                        \
	}

// The only change to the LocalVector class is adding this NEW_VEC_CONSTRUCTORS macro.
#define NEW_VEC_CONSTRUCTORS                                                                                                                                                                 \
	MAKE_VARIANT_CONSTRUCTOR(PackedInt32Array, int32_t, std::is_integral_v<T>)                                                                                                               \
	MAKE_VARIANT_CONSTRUCTOR(PackedFloat32Array, float, std::is_floating_point_v<T>)                                                                                                         \
	MAKE_VARIANT_CONSTRUCTOR_SAME(PackedStringArray, String)                                                                                                                                 \
	MAKE_VARIANT_CONSTRUCTOR_SAME(PackedVector2Array, Vector2)                                                                                                                               \
	MAKE_VARIANT_CONSTRUCTOR_SAME(PackedVector3Array, Vector3)                                                                                                                               \
	MAKE_VARIANT_CONSTRUCTOR_SAME(PackedColorArray, Color)                                                                                                                                   \
	MAKE_VARIANT_CONSTRUCTOR_SAME(PackedVector4Array, Vector4)

// This is the exact same as the godot LocalVector except it has a few more constructors that make it easier to move to and from variant Array types.
// It also has a different Iterator that makes it work with std::algorithms and std::ranges stuff
// Makes loading configs and serialization easier.
template <typename T, typename U = uint32_t, bool force_trivial = false, bool tight = false> class Vec {
	static_assert(!force_trivial, "force_trivial is no longer supported. Use resize_uninitialized instead.");

public:
	NEW_VEC_CONSTRUCTORS
	using value_type = T;

	// Iterator API (satisfies std::ranges::contiguous_range constraints https://stackoverflow.com/a/75061822)
	template <bool IsConst> class Iterator {
	public:
		using difference_type = std::ptrdiff_t;
		using element_type = T;
		using pointer = std::conditional_t<IsConst, const T *, T *>;
		using reference = std::conditional_t<IsConst, const T &, T &>;
		using iterator_concept = std::contiguous_iterator_tag;

		Iterator(pointer p_ptr) :
				elem_ptr(p_ptr) {}
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

private:
	U count = 0;
	U capacity = 0;
	T *data = nullptr;

	template <bool p_init> void _resize(U p_size) {
		if (p_size < count) {
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (U i = p_size; i < count; i++)
					data[i].~T();
			count = p_size;
		} else if (p_size > count) {
			if (unlikely(p_size > capacity)) {
				capacity = tight ? p_size : nearest_power_of_2_templated(p_size);
				data = (T *)memrealloc(data, capacity * sizeof(T));
				CRASH_COND_MSG(!data, "Out of memory");
			}
			if constexpr (p_init)
				memnew_arr_placement(data + count, p_size - count);
			else
				static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible to resize uninitialized");
			count = p_size;
		}
	}

public:
	_FORCE_INLINE_ T *ptr() { return data; }
	_FORCE_INLINE_ const T *ptr() const { return data; }
	_FORCE_INLINE_ U size() const { return count; }

	_FORCE_INLINE_ Span<T> span() const { return Span(data, count); }
	_FORCE_INLINE_ operator Span<T>() const { return span(); }

	// Must take a copy instead of a reference (see GH-31736).
	_FORCE_INLINE_ void push_back(T p_elem) {
		if (unlikely(count == capacity))
			reserve(count + 1);

		memnew_placement(&data[count++], T(std::move(p_elem)));
	}

	void remove_at(U p_index) {
		ERR_FAIL_UNSIGNED_INDEX(p_index, count);
		count--;
		for (U i = p_index; i < count; i++)
			data[i] = std::move(data[i + 1]);
		data[count].~T();
	}

	/// Removes the item copying the last value into the position of the one to
	/// remove. It's generally faster than `remove_at`.
	void remove_at_unordered(U p_index) {
		ERR_FAIL_INDEX(p_index, count);
		count--;
		if (count > p_index)
			data[p_index] = std::move(data[count]);
		data[count].~T();
	}

	_FORCE_INLINE_ bool erase(const T &p_val) {
		int64_t idx = find(p_val);
		if (idx >= 0) {
			remove_at(idx);
			return true;
		}
		return false;
	}

	bool erase_unordered(const T &p_val) {
		int64_t idx = find(p_val);
		if (idx >= 0) {
			remove_at_unordered(idx);
			return true;
		}
		return false;
	}

	U erase_multiple_unordered(const T &p_val) {
		U from = 0;
		U occurrences = 0;
		while (true) {
			int64_t idx = find(p_val, from);

			if (idx == -1)
				break;
			remove_at_unordered(idx);
			from = idx;
			occurrences++;
		}
		return occurrences;
	}

	void reverse() {
		for (U i = 0; i < count / 2; i++)
			SWAP(data[i], data[count - i - 1]);
	}
#ifndef DISABLE_DEPRECATED
	[[deprecated("Use reverse() instead")]] void invert() { reverse(); }
#endif

	_FORCE_INLINE_ void clear() { resize(0); }
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
	void reserve(U p_size) {
		ERR_FAIL_COND_MSG(p_size < size(), "reserve() called with a capacity smaller than the current size. This is likely a mistake.");
		if (p_size > capacity) {
			if (tight) {
				capacity = p_size;
			} else {
				capacity = MAX((U)2, capacity + ((1 + capacity) >> 1));
				if (p_size > capacity)
					capacity = p_size;
			}
			data = (T *)memrealloc(data, capacity * sizeof(T));
			CRASH_COND_MSG(!data, "Out of memory");
		}
	}

	/// Resize the vector.
	/// Elements are initialized (or not) depending on what the default C++ behavior for T is.
	/// Note: If force_trivial is set, this will behave like resize_uninitialized instead.
	void resize(U p_size) {
		// Don't init when trivially constructible.
		_resize<!std::is_trivially_constructible_v<T>>(p_size);
	}

	/// Resize and set all values to 0 / false / nullptr.
	_FORCE_INLINE_ void resize_initialized(U p_size) { _resize<true>(p_size); }

	/// Resize and set all values to 0 / false / nullptr.
	/// This is only available for trivially destructible types (otherwise, trivial resize might be UB).
	_FORCE_INLINE_ void resize_uninitialized(U p_size) { _resize<false>(p_size); }

	_FORCE_INLINE_ const T &operator[](U p_index) const {
		CRASH_BAD_UNSIGNED_INDEX(p_index, count);
		return data[p_index];
	}
	_FORCE_INLINE_ T &operator[](U p_index) {
		CRASH_BAD_UNSIGNED_INDEX(p_index, count);
		return data[p_index];
	}

	void insert(U p_pos, T p_val) {
		ERR_FAIL_UNSIGNED_INDEX(p_pos, count + 1);
		if (p_pos == count) {
			push_back(std::move(p_val));
		} else {
			resize(count + 1);
			for (U i = count - 1; i > p_pos; i--)
				data[i] = std::move(data[i - 1]);
			data[p_pos] = std::move(p_val);
		}
	}

	int64_t find(const T &p_val, int64_t p_from = 0) const {
		if (p_from < 0)
			p_from = size() + p_from;
		if (p_from < 0 || p_from >= size())
			return -1;
		return span().find(p_val, p_from);
	}

	bool has(const T &p_val) const { return find(p_val) != -1; }

	template <typename C> void sort_custom() {
		U len = count;
		if (len == 0)
			return;

		SortArray<T, C> sorter;
		sorter.sort(data, len);
	}

	void sort() { sort_custom<Comparator<T>>(); }

	void ordered_insert(T p_val) {
		U i;
		for (i = 0; i < count; i++)
			if (p_val < data[i])
				break;
		insert(i, p_val);
	}

	operator Vector<T>() const {
		Vector<T> ret;
		ret.resize(count);
		T *w = ret.ptrw();
		if (w) {
			if constexpr (std::is_trivially_copyable_v<T>)
				memcpy(w, data, sizeof(T) * count);
			else
				for (U i = 0; i < count; i++)
					w[i] = data[i];
		}
		return ret;
	}

	Vector<uint8_t> to_byte_array() const { // useful to pass stuff to gpu or variant
		Vector<uint8_t> ret;
		ret.resize(count * sizeof(T));
		uint8_t *w = ret.ptrw();
		if (w)
			memcpy(w, data, sizeof(T) * count);
		return ret;
	}

	_FORCE_INLINE_ Vec() {}
	_FORCE_INLINE_ Vec(std::initializer_list<T> p_init) {
		reserve(p_init.size());
		for (const T &element : p_init)
			push_back(element);
	}
	_FORCE_INLINE_ Vec(const Vec &p_from) {
		resize(p_from.size());
		for (U i = 0; i < p_from.count; i++)
			data[i] = p_from.data[i];
	}
	_FORCE_INLINE_ Vec(Vec &&p_from) {
		data = p_from.data;
		count = p_from.count;
		capacity = p_from.capacity;

		p_from.data = nullptr;
		p_from.count = 0;
		p_from.capacity = 0;
	}

	inline void operator=(const Vec &p_from) {
		resize(p_from.size());
		for (U i = 0; i < p_from.count; i++)
			data[i] = p_from.data[i];
	}
	inline void operator=(const Vector<T> &p_from) {
		resize(p_from.size());
		for (U i = 0; i < count; i++)
			data[i] = p_from[i];
	}
	inline void operator=(Vec &&p_from) {
		if (unlikely(this == &p_from))
			return;
		reset();

		data = p_from.data;
		count = p_from.count;
		capacity = p_from.capacity;

		p_from.data = nullptr;
		p_from.count = 0;
		p_from.capacity = 0;
	}
	inline void operator=(Vector<T> &&p_from) {
		resize(p_from.size());
		for (U i = 0; i < count; i++)
			data[i] = std::move(p_from[i]);
	}

	_FORCE_INLINE_ ~Vec() {
		if (data)
			reset();
	}
};

template <typename T, typename U = uint32_t> using TightVec = Vec<T, U, false, true>;

} // namespace CG
