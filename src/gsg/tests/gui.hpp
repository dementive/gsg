#pragma once
#ifdef TOOLS_ENABLED

#include "scene/gui/control.h"
#include "scene/main/window.h"
#include "SFT.hpp"

namespace CG {

#define test_gui_scene(m_path, m_class) \
	test_scene(m_path, Control, root_node) \
	SceneTree::get_singleton()->get_root()->get_child(0)->add_child(root_node);\
	named_tests( \
		#m_class, \
		"in tree", root_node->is_inside_tree() \
	) \
	test_scene_end(root_node)	

test(gui) {
	test_gui_scene("res://scenes/gui/hud.tscn", Hud)
}

}

#endif