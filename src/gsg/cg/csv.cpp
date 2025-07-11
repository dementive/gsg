#include "csv.hpp"

#include "core/io/file_access.h"

namespace CSV {

namespace {
_ALWAYS_INLINE_ Vector<Variant::Type> determine_types(const Vector<String> &p_values) {
	Vector<Variant::Type> arr{};
	arr.resize(p_values.size());
	Variant::Type *ptr = arr.ptrw();

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

_ALWAYS_INLINE_ Vector<Variant> convert_types(const Vector<String> &p_values, const Vector<Variant::Type> &p_types) {
	Vector<Variant> arr{};
	arr.resize(p_values.size());
	Variant *ptr = arr.ptrw();

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

Vector<Vector<Variant>> parse_file(const String &p_file_name) {
	const Ref<FileAccess> file = FileAccess::open(p_file_name, FileAccess::READ);

	String line = file->get_line(); // skip csv header
	while (line.begins_with("#")) // ignore comments at start of file
		line = file->get_line();

	const Vector<String> first_line_data = file->get_csv_line();
	const Vector<Variant::Type> types = determine_types(first_line_data);

	Vector<Vector<Variant>> data{};
	data.append(convert_types(first_line_data, types));

	while (!file->eof_reached()) {
		const Vector<String> line_strings = file->get_csv_line();
		if (line_strings.size() != types.size()) // ignore lines that are empty, have comments, or are incorrectly formatted.
			continue;
		data.append(convert_types(line_strings, types));
	}

	return data;
}

} // namespace CSV