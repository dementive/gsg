#ifdef TOOLS_ENABLED

#include "Locator.hpp"

#include "core/io/config_file.h"

#include "Registry.hpp"

using namespace CG;

int EditorLocators::get_locator_size(LocatorType p_locator) {
	if (p_locator == LocatorType::Unit) {
		const auto land_provinces_view = Registry::self->view<LandProvinceTag>();
		const auto ocean_provinces_view = Registry::self->view<OceanProvinceTag>();
		const auto river_provinces_view = Registry::self->view<RiverProvinceTag>();

		return land_provinces_view.size() + ocean_provinces_view.size() + river_provinces_view.size();
	}
	return 0;
}

EditorLocators *EditorLocators::self = nullptr;

EditorLocators::EditorLocators() {
	if (self == nullptr) {
		self = this;
		load();
	}
}

EditorLocators::~EditorLocators() { save(); }

String EditorLocators::get_type_string(LocatorType p_type) {
	switch (p_type) {
		case LocatorType::Unit: {
			return "unit";
		} break;
	}
}

void EditorLocators::set_locator(LocatorType p_locator_type, int64_t p_locator_index, const Locator &p_locator) {
	if (p_locator_type == LocatorType::Unit)
		unit_locators[p_locator_index] = p_locator;
}

Locator EditorLocators::get_locator(LocatorType p_locator_type, int p_locator) {
	if (p_locator_type == LocatorType::Unit) {
		if (!unit_locators.has(p_locator))
			ERR_FAIL_V_MSG(Locator(), vformat("Failed to get %s locator %d", get_type_string(p_locator_type), p_locator));
		return unit_locators[p_locator];
	}

	return {};
}

void EditorLocators::load() {
	ConfigFile *unit_config = memnew(ConfigFile());

	if (unit_config->load("res://data/locators/unit.cfg") != OK)
		return;

	List<String> unit_sections;
	unit_config->get_sections(&unit_sections);

	int unit_sections_size = unit_sections.size();
	int unit_locator_excepted_size = get_locator_size(LocatorType::Unit);

	AHashMap<int, Locator> existing_locators;

	// Have to regenerated missing locators.
	if (unit_sections_size != unit_locator_excepted_size) {
		// Get all existing locators
		for (int idx = 1; const String &section : unit_sections) {
			Locator locator;
			locator.position = unit_config->get_value(section, "position");
			locator.orientation = unit_config->get_value(section, "orientation");
			locator.scale = unit_config->get_value(section, "scale");
			existing_locators[idx] = locator;
			idx++;
		}

		for (int i = 1; i < unit_locator_excepted_size + 1; ++i) {
			if (existing_locators.has(i)) {
				// Copy existing locator
				unit_locators[i] = existing_locators[i];
			} else {
				// Generate new locator
				// For unit locators only need to generate the position but for text need to also generate the orientation.
				Locator locator;
				const auto entity = Registry::self->get_entity<ProvinceTag>(i);
				locator.position = Registry::self->get<Centroid>(entity);
				// locator.orientation = Registry::self->get<Orientation>(entity);
				locator.orientation = 0.0;
				locator.scale = 1.0;
				unit_locators[i] = locator;
			}
		}
	} else {
		// All locators exist so can just load them
		for (const String &section : unit_sections) {
			Locator locator;
			locator.position = unit_config->get_value(section, "position");
			locator.orientation = unit_config->get_value(section, "orientation");
			locator.scale = unit_config->get_value(section, "scale");
			unit_locators[section.to_int()] = locator;
		}
	}
}

void EditorLocators::save() {
	const Ref<ConfigFile> config = memnew(ConfigFile());

	for (const auto &locator : unit_locators) {
		config->set_value(itos(locator.key), "position", unit_locators[locator.key].position);
		config->set_value(itos(locator.key), "scale", unit_locators[locator.key].scale);
		config->set_value(itos(locator.key), "orientation", unit_locators[locator.key].orientation);
	}

	config->save("res://data/locators/unit.cfg");
}

#endif
