#include "Locator.hpp"

#include "ecs/tags.hpp"

using namespace CG;

bool Locator::operator==(const Locator &other) const { return position == other.position && orientation == other.orientation && scale == other.scale; }
bool Locator::operator!=(const Locator &other) const { return position != other.position || orientation != other.orientation || scale != other.scale; }

#ifdef TOOLS_ENABLED

#include "core/io/config_file.h"

Vec<Entity> EditorLocators::get_locator_vec(LocatorType p_locator) {
	const auto land_provinces_view = ECS::self->query<LandProvinceTag>();
	const auto ocean_provinces_view = ECS::self->query<OceanProvinceTag>();
	const auto river_provinces_view = ECS::self->query<RiverProvinceTag>();
	Vec<Entity> province_ids;

	switch (p_locator) {
		case LocatorType::Unit: {
			province_ids.reserve(land_provinces_view.count() + ocean_provinces_view.count() + river_provinces_view.count());
			land_provinces_view.each([&province_ids](Entity p_entity, LandProvinceTag) { province_ids.push_back(p_entity); });
			ocean_provinces_view.each([&province_ids](Entity p_entity, OceanProvinceTag) { province_ids.push_back(p_entity); });
			river_provinces_view.each([&province_ids](Entity p_entity, RiverProvinceTag) { province_ids.push_back(p_entity); });
			return province_ids;
		} break;
		case LocatorType::Text: {
			province_ids.reserve(land_provinces_view.count());
			land_provinces_view.each([&province_ids](Entity p_entity, LandProvinceTag) { province_ids.push_back(p_entity); });
			return province_ids;
		} break;
	}

	return province_ids;
}

EditorLocators *EditorLocators::self = nullptr;

EditorLocators::EditorLocators() {
	if (self == nullptr) {
		self = this;
		load();
	}
}

EditorLocators::~EditorLocators() { save_all(); }

String EditorLocators::get_type_string(LocatorType p_type) {
	switch (p_type) {
		case LocatorType::Unit: {
			return "unit";
		} break;
		case LocatorType::Text: {
			return "text";
		} break;
	}

	return "";
}

void EditorLocators::set_locator(LocatorType p_locator_type, int64_t p_locator_index, const Locator &p_locator) {
	switch (p_locator_type) {
		case LocatorType::Unit: {
			unit_locators[p_locator_index] = p_locator;
		} break;
		case LocatorType::Text: {
			text_locators[p_locator_index] = p_locator;
		} break;
	}

	set_dirty(p_locator_type);
}

Locator EditorLocators::get_locator(LocatorType p_locator_type, int p_locator) {
	switch (p_locator_type) {
		case LocatorType::Unit: {
			ERR_FAIL_COND_V_MSG(!unit_locators.has(p_locator), Locator(), String("Failed to get " + get_type_string(p_locator_type) + " locator " + p_locator));
			return unit_locators[p_locator];
		} break;
		case LocatorType::Text: {
			ERR_FAIL_COND_V_MSG(!text_locators.has(p_locator), Locator(), String("Failed to get " + get_type_string(p_locator_type) + " locator " + p_locator));
			return text_locators[p_locator];
		} break;
	}

	return {};
}

bool EditorLocators::has_locator(LocatorType p_locator_type, int p_province_id) {
	switch (p_locator_type) {
		case LocatorType::Unit: {
			return unit_locators.has(p_province_id);
		} break;
		case LocatorType::Text: {
			return text_locators.has(p_province_id);
		} break;
	}

	return false;
}

static constexpr float DEFAULT_LOCATOR_SCALE = 25.0;
static constexpr float DEFAULT_TEXT_LOCATOR_SCALE = 100.0;

void EditorLocators::_load_locators(LocatorMap &p_locator_map, const String &p_cfg_path, LocatorType p_locator) {
	const Ref<ConfigFile> config = memnew(ConfigFile());

	if (config->load(p_cfg_path) != OK)
		return;

	const Vector<String> sections = config->get_sections();

	const uint32_t sections_size = sections.size();
	const Vec<Entity> province_ids = get_locator_vec(p_locator);

	AHashMap<int, Locator> existing_locators;
	const Ref<ConfigFile> province_data_config = memnew(ConfigFile());
	province_data_config->load("res://data/gen/province_data.cfg");

	// Have to regenerated missing locators.
	if (sections_size != province_ids.size()) {
		// Get all existing locators
		for (const String &section : sections) {
			Locator locator;
			locator.position = config->get_value(section, "position");
			locator.orientation = config->get_value(section, "orientation");
			locator.scale = config->get_value(section, "scale");
			existing_locators[section.to_int()] = locator;
		}

		for (const Entity entity : province_ids) {
			const int i = int(entity) + 1;
			if (existing_locators.has(i)) {
				// Copy existing locator
				p_locator_map[i] = existing_locators[i];
			} else {
				// Generate new locator
				Locator locator;
				locator.position = province_data_config->get_value(itos(i), "centroid");

				// For unit locators only need to generate the position but for text need to also generate the orientation.
				if (p_locator == LocatorType::Text)
					locator.orientation = province_data_config->get_value(itos(i), "orientation");
				else
					locator.orientation = 0.0;

				if (p_locator == LocatorType::Unit)
					locator.scale = DEFAULT_LOCATOR_SCALE;
				else if (p_locator == LocatorType::Text)
					locator.scale = DEFAULT_TEXT_LOCATOR_SCALE;
				p_locator_map[i] = locator;
			}
		}
	} else {
		// All locators exist so can just load them
		for (const String &section : sections) {
			Locator locator;
			locator.position = config->get_value(section, "position");
			locator.orientation = config->get_value(section, "orientation");
			locator.scale = config->get_value(section, "scale");
			p_locator_map[section.to_int()] = locator;
		}
	}
}

void EditorLocators::load() {
	_load_locators(unit_locators, "res://data/locators/unit.cfg", LocatorType::Unit);
	_load_locators(text_locators, "res://data/locators/text.cfg", LocatorType::Text);
}

void EditorLocators::_save_config(const LocatorMap &p_locator_map, const String &p_cfg_path, LocatorType p_locator_type) {
	switch (p_locator_type) {
		case LocatorType::Unit: {
			if (save_flags.unit == false)
				return;
			else
				save_flags.unit = false;
		} break;
		case LocatorType::Text: {
			if (save_flags.text == false)
				return;
			else
				save_flags.text = false;
		} break;
	}

	const Ref<ConfigFile> config = memnew(ConfigFile());

	for (const auto &locator : p_locator_map) {
		config->set_value(itos(locator.key), "position", locator.value.position);
		config->set_value(itos(locator.key), "scale", locator.value.scale);
		config->set_value(itos(locator.key), "orientation", locator.value.orientation);
	}

	config->save(p_cfg_path);
}

void EditorLocators::save(LocatorType p_locator_type) {
	switch (p_locator_type) {
		case LocatorType::Unit: {
			_save_config(unit_locators, "res://data/locators/unit.cfg", p_locator_type);
		} break;
		case LocatorType::Text: {
			_save_config(text_locators, "res://data/locators/text.cfg", p_locator_type);
		} break;
	}
}

void EditorLocators::save_all() {
	_save_config(unit_locators, "res://data/locators/unit.cfg", LocatorType::Unit);
	_save_config(text_locators, "res://data/locators/text.cfg", LocatorType::Text);
}

void EditorLocators::set_dirty(LocatorType p_locator_type) {
	switch (p_locator_type) {
		case LocatorType::Unit: {
			save_flags.unit = true;
		} break;
		case LocatorType::Text: {
			save_flags.text = true;
		} break;
	}
}

#endif
