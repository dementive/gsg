#ifndef ECS_REGISTER_TYPES_H
#define ECS_REGISTER_TYPES_H

#include "modules/register_module_types.h"

void initialize_flecs_module(ModuleInitializationLevel p_level);
void uninitialize_flecs_module(ModuleInitializationLevel p_level);

#endif // ECS_REGISTER_TYPES_H
