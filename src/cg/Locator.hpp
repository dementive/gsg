#pragma once

#ifdef TOOLS_ENABLED

#include "core/templates/a_hash_map.h"

#include "Entity.hpp"
#include "Vec.hpp"

class ConfigFile;

namespace CG {

struct Locator {
	Vector2 position;
	float orientation{};
	float scale = 1.0;

	bool operator!=(const Locator &other) const;
	bool operator==(const Locator &other) const;
};

enum class LocatorType : uint8_t { Unit, Text };

// Class to handle generation and editing of locators in the editor.
struct EditorLocators {
private:
	struct LocatorSaveFlags {
		bool unit : 1 = false;
		bool text : 1 = false;
	};
	using LocatorMap = AHashMap<int, Locator>;
	LocatorMap unit_locators;
	LocatorMap text_locators;
	LocatorSaveFlags save_flags;

	static Vec<Entity> get_locator_vec(LocatorType p_locator);
	static String get_type_string(LocatorType p_type);
	void _load_locators(LocatorMap &p_locator_map, const String &p_cfg_path, LocatorType p_locator);
	void _save_config(const LocatorMap &p_locator_map, const String &p_cfg_path, LocatorType p_locator_type);

public:
	EditorLocators();
	~EditorLocators();
	static EditorLocators *self;

	void set_locator(LocatorType p_locator_type, int64_t p_locator_index, const Locator &p_locator);
	Locator get_locator(LocatorType p_locator_type, int p_locator);
	bool has_locator(LocatorType p_locator_type, int p_province_id);

	void load();
	void set_dirty(LocatorType p_locator_type);
	void save(LocatorType p_locator_type);
	void save_all();
};

} // namespace CG

#endif
