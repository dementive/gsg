#pragma once

#ifdef TOOLS_ENABLED

#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/templates/a_hash_map.h"

class ConfigFile;

namespace CG {

struct Locator {
	Vector2 position;
	float orientation{};
	float scale = 1.0;

	bool operator!=(const Locator &other) const;
	bool operator==(const Locator &other) const;
};

enum class LocatorType : uint8_t { Unit };

// Class to handle generation and editing of locators in the editor.
struct EditorLocators {
private:
	AHashMap<int, Locator> unit_locators;
	static int get_locator_size(LocatorType p_locator);
	static String get_type_string(LocatorType p_type);

public:
	EditorLocators();
	~EditorLocators();
	static EditorLocators *self;

	void set_locator(LocatorType p_locator_type, int64_t p_locator_index, const Locator &p_locator);
	Locator get_locator(LocatorType p_locator_type, int p_locator);

	void load();
	void save();
};

} // namespace CG

#endif
