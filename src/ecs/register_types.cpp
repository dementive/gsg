#include "register_types.h"

#include "core/string/print_string.h"

#include "flecs/distr/flecs.h"

struct ecs {
	static inline ecs *self = nullptr;

	ecs_world_t *world{};
	ecs() {
		if (self == nullptr) {
			print_line("Hello ecs module!");
			world = ecs_init();
			self = this;
		}
	}

	~ecs() { ecs_fini(world); }
};

void initialize_ecs_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	new ecs;
}

void uninitialize_ecs_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	delete ecs::self;
}
