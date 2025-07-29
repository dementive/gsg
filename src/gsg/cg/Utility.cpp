#include "Utility.hpp"

#include "scene/main/node.h"
#include "scene/resources/packed_scene.h"
#include "core/string/translation_server.h"

using namespace CG;

Node *CG::init_scene(const String &p_path) {
	const Ref<PackedScene> scene = ResourceLoader::load(p_path);
	ERR_FAIL_COND_V_MSG(scene == nullptr, nullptr, String("Error initializing:" + p_path));
	return scene->instantiate();
}

StringName CG::translate(const StringName &p_message, const StringName &p_context) {
	return TranslationServer::get_singleton()->get_main_domain()->translate(p_message, p_context);
}
