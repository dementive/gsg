extends MainLoop

var preprocessor_statements: Array[String] = ["#if", "#else", "#endif", "#include"]

var constexpr_var_pattern: RegEx
var consteval_var_pattern: RegEx

var memnew_pattern: RegEx
var memfree_pattern: RegEx
var memqueue_free_pattern: RegEx

var static_type_func_no_equals_pattern: RegEx
var annotation_pattern: RegEx
var static_type_pattern: RegEx

var static_type_no_equals_pattern: RegEx
var type_no_equals_pattern: RegEx
var typed_for_pattern: RegEx
var type_alias_pattern: RegEx
var macro_pattern: RegEx

var is_inside_open_paren: bool = false
var is_compilation_enabled: bool = true

var macro_line_continuation: bool = false
var macro_line_continuation_first: bool = false

var current_macro_value: String = ""
var current_macro_key: String = ""

var is_compilation_enabled_stack: Array[bool] = []

var macro_dict: Dictionary[String, Variant] = {}
var constexpr_variables: Dictionary[String, Variant] = {}
var consteval_variables: Dictionary[String, Variant] = {}
var type_aliases: Dictionary[String, String] = {}

func whitespace_split(p_string: String, p_splitter: String = " ") -> PackedStringArray:
	var ret: PackedStringArray
	var inside_quote: bool = false
	var quote_type: bool = false
	var current_word: String
	var comment_started: bool = false

	for i in p_string.length():
		if (p_string[i] == '#' && !inside_quote && !comment_started):
			comment_started = true
		elif (p_string[i] == '\n' && comment_started):
			comment_started = false
		elif (!comment_started):
			if (p_string[i] == '"' && !inside_quote):
				inside_quote = true
				quote_type = true
				current_word += p_string[i]
			elif (p_string[i] == '"' && inside_quote && quote_type):
				inside_quote = false
				current_word += p_string[i]
			elif (p_string[i] == '\'' && !inside_quote):
				inside_quote = true
				quote_type = false
				current_word += p_string[i]
			elif (p_string[i] == '\'' && inside_quote && !quote_type):
				inside_quote = false
				current_word += p_string[i]
			elif (p_string[i] == p_splitter && !inside_quote):
				if (!current_word.is_empty()):
					ret.push_back(current_word)
					current_word = ""
			else:
				current_word += p_string[i]

	if (!current_word.is_empty()):
		ret.push_back(current_word)

	return ret

func compile_regex() -> void:
	var var_pattern: String = r"([A-Za-z_0-9]\w*)"
	var type_pattern: String = r"([A-Za-z_0-9]\w*(?:\[[^\[\]]+\])?)"
	var simple_keyword_pattern_fmt: String = r"%s\s+%s"

	var var_declaration_pattern_fmt: String = r"%s\s+%s\s+=(.*)"
	var var_declaration_pattern: String = var_declaration_pattern_fmt % [type_pattern, var_pattern]

	var constant_expression_fmt = r"%s\s+%s"
	constexpr_var_pattern = RegEx.new()
	constexpr_var_pattern.compile(constant_expression_fmt % ["constexpr", var_declaration_pattern])

	consteval_var_pattern = RegEx.new()
	consteval_var_pattern.compile(constant_expression_fmt % ["consteval", var_declaration_pattern])

	memnew_pattern = RegEx.new()	
	memnew_pattern.compile(simple_keyword_pattern_fmt % ["new", var_pattern])

	memfree_pattern = RegEx.new()		
	memfree_pattern.compile(simple_keyword_pattern_fmt % ["free", var_pattern])

	memqueue_free_pattern = RegEx.new()		
	memqueue_free_pattern.compile(simple_keyword_pattern_fmt % ["qfree", var_pattern])

	static_type_func_no_equals_pattern = RegEx.new()
	static_type_func_no_equals_pattern.compile(simple_keyword_pattern_fmt % [type_pattern, var_pattern])

	annotation_pattern = RegEx.new()
	annotation_pattern.compile(r"^(export|onready)\s+{type_pattern}\s+{var_pattern}(.*)".format({"type_pattern": type_pattern, "var_pattern": var_pattern}))

	static_type_pattern = RegEx.new()
	static_type_pattern.compile(var_declaration_pattern)

	type_no_equals_pattern = RegEx.new()
	type_no_equals_pattern.compile(r"{type_pattern}\s+{var_pattern}\n".format({"type_pattern": type_pattern, "var_pattern": var_pattern}))

	typed_for_pattern = RegEx.new()
	typed_for_pattern.compile(r"for\s+{type_pattern}\s+{var_pattern}".format({"type_pattern": type_pattern, "var_pattern": var_pattern}))

	type_alias_pattern = RegEx.new()
	type_alias_pattern.compile(r"using\s+{type_pattern}\s+=\s+{type_pattern}".format({"type_pattern": type_pattern}))

func exec_and_return(expression: String) -> Variant: 
	var expr := Expression.new()
	expr.parse(expression)
	var result: Variant = expr.execute() 
	if (expr.has_execute_failed()):
		print("Error executing expression: ", expression)
	return result

func replace_consteval_vars(line_tokens: PackedStringArray) -> PackedStringArray:
	for variable in consteval_variables:
		var iter_idx: int = 0
		for token in line_tokens:
			if token == variable:
				line_tokens[iter_idx] = type_convert(consteval_variables[variable], TYPE_STRING)
			iter_idx += 1

	return line_tokens

func replace_constexpr_vars(line_tokens: PackedStringArray) -> PackedStringArray:
	for variable in constexpr_variables:
		var iter_idx: int = 0
		for token in line_tokens:
			if token == variable:
				line_tokens[iter_idx] = type_convert(constexpr_variables[variable], TYPE_STRING)
			iter_idx += 1

	return line_tokens

func get_indentation_level(line: String) -> int:
	var level: int = 0
	for char in line:
		if char == "\t" or char == "    ":
			level += 1
		else:
			break

	return level

func process_line(line: String, file: FileAccess) -> void:
	if macro_line_continuation:
		if macro_line_continuation_first:
			macro_line_continuation_first = false
			line = line.lstrip("\t")
		if not line.is_empty():
			current_macro_value += line + "\n"
		if not line.ends_with("\\"):
			macro_line_continuation = false
			macro_dict[current_macro_key] = current_macro_value.replace("\\", "")
			current_macro_value = ""
		return

	if line.begins_with("#define"):
		var macro_tokens: PackedStringArray = line.split(" ")
		macro_tokens.remove_at(0)
		current_macro_key = macro_tokens[0]
		current_macro_value = macro_tokens[1]

		if current_macro_value.ends_with("\\"):
			macro_line_continuation = true
			macro_line_continuation_first = true
		elif not current_macro_value.is_empty():
			macro_dict[current_macro_key] = current_macro_value

		return

	var indentation_level: int = get_indentation_level(line)
	line = line.strip_edges()
	var line_tokens: PackedStringArray = whitespace_split(line)

	# Replace consteval variables with their values
	line_tokens = replace_consteval_vars(line_tokens)
	for variable in consteval_variables:
		line = line.replacen(variable, type_convert(consteval_variables[variable], TYPE_STRING))

	if line.begins_with("#if"):
		line_tokens = replace_constexpr_vars(line.split(" "))
		line_tokens.remove_at(0)

		var expression: String = " ".join(line_tokens)
		var expression_result = exec_and_return(expression)
		if (expression_result is bool or expression_result is int):
			is_compilation_enabled = expression_result
			is_compilation_enabled_stack.append(is_compilation_enabled)
		else:
			print("Wrong type for #if preprocessor statement")

		return

	if line.begins_with("#else"):
		is_compilation_enabled = not is_compilation_enabled
		return

	if line.begins_with("#endif"):
		if is_compilation_enabled_stack.size() > 0:
			is_compilation_enabled = is_compilation_enabled_stack.pop_back()
		else:
			is_compilation_enabled = true

		if is_compilation_enabled_stack.size() <= 0:
			is_compilation_enabled = true

		return

	if line.begins_with("using"):
		var alias_var_match: RegExMatch = type_alias_pattern.search(line)
		if alias_var_match:
			var groups: PackedStringArray = alias_var_match.strings
			type_aliases[groups[1]] = groups[2]
		else:
			print("Invalid type alias on line: ", line)
			print_stack()

		return

	if line.begins_with("#"):
		file.store_line(line)
		return


	var iter_idx: int = 0
	for token in line_tokens:
		for alias in type_aliases:
			if token == alias:
				line_tokens[iter_idx] = type_convert(type_aliases[alias], TYPE_STRING)

		for macro in macro_dict:
			if token == macro:
				line_tokens[iter_idx] = type_convert(macro_dict[macro], TYPE_STRING)
		
		iter_idx += 1


	var line_split: PackedStringArray = line.split("#")
	var comment_string: String = ""
	if line_split.size() > 1:
		comment_string = line_split[1]

	if (line_tokens.is_empty()):
		file.store_line("")
		return

	var first_token: String = line_tokens[0]
	var last_token: String = line_tokens[line_tokens.size() - 1]

	if not is_compilation_enabled:
		return

	if first_token == "fn":
		line_tokens[0] = "func"
		first_token = "func"

	if (first_token == "static" and line_tokens[1] == "fn"):
		line_tokens[1] = "func"

	# Constexpr variables
	if first_token == "constexpr":
		line_tokens = replace_constexpr_vars(line_tokens)
		var line_str: String = " ".join(line_tokens)

		var constexpr_var_match: RegExMatch = constexpr_var_pattern.search(line_str)
		if !constexpr_var_match:
			print("Compile error on line: ", line)
			print_stack()
			return

		var groups: PackedStringArray = constexpr_var_match.strings
		var var_name: String = groups[2]
		var expression: String = groups[3].strip_edges()
		var expression_result: Variant = exec_and_return(expression)
		constexpr_variables[var_name] = expression_result

		var idx: int = 0
		var should_replace: bool = false
		var new_tokens: PackedStringArray
		for token in line_tokens:
			if !should_replace and token == "=":
				should_replace = true
				new_tokens.append(token)
			if token == "constexpr":
				token = "const"
			if !should_replace:
				new_tokens.append(token)

			idx += 1

		line_tokens = new_tokens
		line_tokens.append(str(expression_result))

	if first_token == "consteval":
		line_tokens = replace_constexpr_vars(line_tokens)

		var line_str: String = " ".join(line_tokens)
		var consteval_var_match: RegExMatch = consteval_var_pattern.search(line_str)
		if !consteval_var_match:
			print("Compile error on line: ", line)
			print_stack()
			return

		var groups: PackedStringArray = consteval_var_match.strings
		var var_name: String = groups[2]
		var expression: String = groups[3].strip_edges()
		var expression_result: Variant = exec_and_return(expression)
		consteval_variables[var_name] = expression_result

		return

	if last_token == '(':
		is_inside_open_paren = true

	if is_inside_open_paren and (last_token == ')' or last_token == '):'):
		is_inside_open_paren = false

	var line_str = " ".join(line_tokens)

	# Typed function params
	if first_token == "func":
		var no_equals_match: Array[RegExMatch] = static_type_func_no_equals_pattern.search_all(line_str)
		if no_equals_match.size() > 1:
			for match: RegExMatch in no_equals_match:
				var groups: PackedStringArray = match.strings
				if not groups[1].begins_with("func") and not groups[1].begins_with("static"):
					line_str = line_str.replace("%s %s" % [groups[1], groups[2]], "%s: %s" % [groups[2], groups[1]])

	var pattern_match: RegExMatch = memnew_pattern.search(line_str)
	if pattern_match:
		line_str = memnew_pattern.sub(line_str, r"$1.new()")  # Allow using "new Object" syntax in place of "Object.new()"

	pattern_match = memqueue_free_pattern.search(line_str)
	if pattern_match:
		line_str = memqueue_free_pattern.sub(line_str, r"$1.queue_free()")

	pattern_match = memfree_pattern.search(line_str)
	if pattern_match:
		line_str = memfree_pattern.sub(line_str, r"$1.free()")

	line_tokens = whitespace_split(line_str)
	line_str = " ".join(line_tokens)
	first_token = line_tokens[0]
	last_token = line_tokens[line_tokens.size() - 1]

	# onready and export annotations
	var annotation_match: RegExMatch = annotation_pattern.search(line_str)
	if annotation_match:
		line_str = annotation_pattern.sub(line_str, r"@$1 var $3: $2$4")

	# Variable declarations
	var static_type_var_declaration_match: RegExMatch = static_type_pattern.search(line_str)
	if static_type_var_declaration_match:
		var static_type_var_groups: PackedStringArray = static_type_var_declaration_match.strings
		if (static_type_var_declaration_match and not static_type_var_groups[1] in ["var", "const"]
			and not (first_token == "func" or first_token == "static func")
		):
			line_str = static_type_pattern.sub(line_str, r"var $2: $1 =$3")

	# Variable definitions
	var no_eq_line_str: String = line_str + "\n"
	var no_equals_match: RegExMatch = type_no_equals_pattern.search(no_eq_line_str)
	if no_equals_match and not (first_token == "return" or first_token == "extends" or first_token == "class_name" or first_token == "func"):
		line_str = type_no_equals_pattern.sub(no_eq_line_str, r"var $2: $1")

	if first_token == "const":
		line_str = line_str.replace("const var", "const")

	# Typed for loop
	var typed_for_match: RegExMatch = typed_for_pattern.search(line_str)
	if typed_for_match:
		var typed_for_match_groups: PackedStringArray = typed_for_match.strings
		if typed_for_match and typed_for_match_groups[2] != "in":
			line_str = typed_for_pattern.sub(line_str, r"for $2: $1")

	for level in range(indentation_level):
		line_str = "\t" + line_str

	if comment_string:
		line_str += " #" + comment_string

	file.store_line(line_str)


func compile(input_file_name: String, output_file_name: String) -> void: 
	var file := FileAccess.open(input_file_name, FileAccess.READ)
	var out_file := FileAccess.open(output_file_name, FileAccess.WRITE)
	while not file.eof_reached():
		var line: String = file.get_line()

		if line.strip_edges().begins_with("#include"):
			var include_path: PackedStringArray = line.split("#include")
			var include_path_str: String = include_path[1].strip_edges().replace('"', "")

			var include_file := FileAccess.open(include_path_str, FileAccess.READ)
			line = include_file.get_as_text()
			var lines: PackedStringArray = line.split("\n")
			for included_line in lines:
				process_line(included_line, out_file)

		process_line(line, out_file)
	file.close()

func _init(): 
	compile_regex()
	compile("test.gdp", "output.gd")
