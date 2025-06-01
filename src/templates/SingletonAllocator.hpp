#pragma once

#include "core/os/memory.h"

namespace CG {

// Allocator for singletons so they can get freed together easily and they are next to each other in memory.
template <typename... Args> class SingletonAllocator {
private:
	void *_data{};
	uint32_t total_bytes{};

	template <typename T> void init_singletons() {
		new (static_cast<char *>(_data) + total_bytes) T();
		total_bytes += sizeof(T);
	}

	template <typename T> void free_singletons() { T::self->~T(); }

	static consteval size_t total_size() { return (sizeof(Args) + ...); }

public:
	void init() {
		_data = memalloc(total_size());
		(init_singletons<Args>(), ...);
	}

	void free() {
		// Call destructors for all allocated objects
		(free_singletons<Args>(), ...);
		memfree(_data);
	}
};

} // namespace CG
