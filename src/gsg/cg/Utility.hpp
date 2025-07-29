#pragma once

#include "core/string/string_name.h"

class Node;

namespace CG {

Node *init_scene(const String &p_path);
StringName translate(const StringName &p_message, const StringName &p_context = "");

} // namespace CG
