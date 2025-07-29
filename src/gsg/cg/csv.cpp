#include "csv.hpp"

#include "core/io/file_access.h"

namespace CSV {

namespace {
_ALWAYS_INLINE_ LocalVector<Variant::Type> determine_types(const Vector<String> &p_values) {
	LocalVector<Variant::Type> arr{};
	arr.resize(p_values.size());
	Variant::Type *ptr = arr.ptr();

	for (int i = 0; const String &value : p_values) {
		if (value.is_valid_int())
			ptr[i] = Variant::Type::INT;
		else if (value.is_valid_float())
			ptr[i] = Variant::Type::FLOAT;
		else
			ptr[i] = Variant::Type::STRING;
		i++;
	}

	return arr;
}

_ALWAYS_INLINE_ Line convert_types(const Vector<String> &p_values, const LocalVector<Variant::Type> &p_types) {
	Line arr{};
	arr.resize(p_values.size());
	Variant *ptr = arr.ptr();

	for (int i = 0; const String &value : p_values) {
		const Variant::Type type = p_types[i];
		Variant new_value = value;

		if (type == Variant::Type::INT)
			new_value = value.to_int();
		else if (type == Variant::Type::FLOAT)
			new_value = value.to_float();

		ptr[i] = new_value;
		i++;
	}

	return arr;
}
} // namespace

LocalVector<Line> parse_file(const String &p_file_name) {
	const Ref<FileAccess> file = FileAccess::open(p_file_name, FileAccess::READ);

	String line = file->get_line(); // skip csv header
	while (line.begins_with("#")) // ignore comments at start of file
		line = file->get_line();

	const Vector<String> first_line_data = file->get_csv_line();
	const LocalVector<Variant::Type> types = determine_types(first_line_data);

	LocalVector<Line> data{};
	data.push_back(convert_types(first_line_data, types));

	while (!file->eof_reached()) {
		const Vector<String> line_strings = file->get_csv_line();
		if (line_strings.size() != types.size()) // ignore lines that are empty, have comments, or are incorrectly formatted.
			continue;
		data.push_back(convert_types(line_strings, types));
	}

	return data;
}

} // namespace CSV