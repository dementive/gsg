#include "Registry.hpp"
#include "core/string/print_string.h"

using namespace CG;

static void update(Registry &registry) {
    auto view = registry.view<const Name>();

    for(auto [entity, name]: view.each()) {
    	print_line(vformat("Entity: %d name: %s", static_cast<int>(entity), name));
    }

    auto provinces_view = registry.view<ProvinceTag>();
    for(const ProvinceEntity entity: provinces_view) {
    	print_line(vformat("Territory Entity: %d name: %s", static_cast<int>(entity), registry.get<Name>(entity)));
    }

    auto provinces_name_view = registry.view<ProvinceTag, Name>();
    for(auto [entity, name]: provinces_name_view.each()) {
    	print_line(vformat("Entity: %d name: %s", static_cast<int>(entity), name));
    }
}

inline void entt_example() {
	// Using different storage type for components
	entt::registry reg;

	auto &&region_storage = reg.storage<Name>("Region"_hs);

	auto storage_entity = reg.create();
	reg.emplace<Name>(storage_entity);
	storage_entity = reg.create();
	region_storage.push(storage_entity);

	auto global_view = reg.view<const Name>();
	entt::basic_view region_view{reg.storage<Name>("Region"_hs)};

	for(auto [entity, name]: global_view.each()) {
		print_line(vformat("Global Entity: %d name: %s", static_cast<int>(entity), name));
	}

	for(auto [entity, name]: region_view.each()) {
		print_line(vformat("Region Entity: %d name: %s", static_cast<int>(entity), name));
	}

	// Usage example
	Registry *registry = new Registry;

	// create provinces
	for(int i = 0; i < 10; ++i) {
		const ProvinceEntity entity = registry->create_entity<ProvinceTag>(i);
		registry->emplace<Name>(entity, "Ohio");
	}

	// create nations
	for(int i = 0; i < 10; ++i) {
		const CountryEntity entity = registry->create_entity<CountryTag>(i);
		registry->emplace<Name>(entity, "Rome");
	}

	auto provinces_name_view = registry->view<ProvinceTag, Name>();

	// Check if all province names are "Rome"
	if (std::ranges::all_of(provinces_name_view, [&](ProvinceEntity p_entity) {
		return provinces_name_view.get<Name>(p_entity) == "Ohio";
	})) {
		print_line("All Ohio\n");
	} else {
		print_line("Not all are Ohio\n");
	}

	// Find the first province with the name "Ohio"
	String search_string = "Ohio";
	auto it_found = std::ranges::find_if(provinces_name_view, [&](ProvinceEntity p_entity) {
		return provinces_name_view.get<Name>(p_entity) == search_string;
	});

	if (it_found != provinces_name_view.end()) {
		print_line("Found a province named Ohio\n");
	} else {
		print_line("No province named Ohio found\n");
	}

	// Check if any province is named "California"
	bool any_california = std::ranges::any_of(provinces_name_view, [&](ProvinceEntity p_entity) {
		return provinces_name_view.get<Name>(p_entity) == "California";
	});

	// Count how many provinces are named "Ohio"
	auto count_ohio = std::ranges::count_if(provinces_name_view, [&](ProvinceEntity p_entity) {
		return provinces_name_view.get<Name>(p_entity) == "Ohio";
	});

	print_line("Number of provinces named Ohio: ", count_ohio, "\n");
	print_line("Cali? : ", any_california, "\n");

	for(auto [entity, name]: provinces_name_view.each()) {
		print_line(vformat("Entity: %d name: %s", static_cast<int>(entity), name));
	}

	auto provinces_name_group = registry->group<Name>();
	for(auto [entity, name]: provinces_name_group.each()) {
		print_line(vformat("Entity: %d name: %s", static_cast<int>(entity), name));
	}


	update(*registry);

	delete registry;
}