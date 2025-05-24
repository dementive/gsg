extends RefCounted
class_name CSV

# Parses a csv file into an Array of Arrays, each sub array is the values on each line.
# If the types of the values are ints or floats they will be automatically converted to the correct type.
static func parse_file(p_file_name: String) -> Array[Array]:
	var file: FileAccess = FileAccess.open(p_file_name, FileAccess.READ)

	var line: String = file.get_line() # skip csv header
	while line.begins_with("#"): # ignore comments at start of file
		line = file.get_line()

	var first_line_data: PackedStringArray = file.get_csv_line()
	var types: Array[Variant.Type] = determine_types(first_line_data)
	var start_data: Array = convert_types(first_line_data, types)
	var data: Array[Array] = []
	data.append(start_data)

	while !file.eof_reached():
		var line_strings: PackedStringArray = file.get_csv_line()
		if line_strings.size() != types.size(): # ignore empty lines and comments
			continue
		data.append(convert_types(line_strings, types))

	return data

# Check first csv line to figure out types of each column
static func determine_types(p_values: PackedStringArray) -> Array[Variant.Type]:
	var arr: Array[Variant.Type] = []
	arr.resize(p_values.size())
	
	var i: int = 0
	for value: String in p_values:
		if value.is_valid_int():
			arr[i] = Variant.Type.TYPE_INT
		elif value.is_valid_float():
			arr[i] = Variant.Type.TYPE_FLOAT
		else:
			arr[i] = Variant.Type.TYPE_STRING

		i += 1
	
	return arr

static func convert_types(p_values: PackedStringArray, p_types: Array[Variant.Type]) -> Array:
	var arr: Array = []
	arr.resize(p_values.size())
	var i: int = 0
	for value: String in p_values:
		var type: Variant.Type = p_types[i]
		var new_value: Variant = value
		if type == Variant.Type.TYPE_INT:
			new_value = value.to_int()
		elif type == Variant.Type.TYPE_FLOAT:
			new_value = value.to_float()
		arr[i] = new_value

		i += 1
	return arr
