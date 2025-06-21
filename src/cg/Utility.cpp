#include "Utility.hpp"

#include "scene/main/node.h"
#include "scene/resources/packed_scene.h"

using namespace CG;

Node *UT::init_scene(const String &p_path) {
	const Ref<PackedScene> scene = ResourceLoader::load(p_path);
	ERR_FAIL_COND_V_MSG(scene == nullptr, nullptr, String("Error initializing:" + p_path));
	return scene->instantiate();
}
