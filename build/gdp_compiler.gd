extends MainLoop

var preprocessor_statements: Array[String] = ["#if", "#else", "#endif", "#include"]

var block_comment_pattern: RegEx
var constexpr_var_pattern: RegEx
var consteval_var_pattern: RegEx

var memnew_pattern: RegEx
var memfree_pattern: RegEx
var memqueue_free_pattern: RegEx

var static_type_func_no_equals_pattern: RegEx
var annotation_pattern: RegEx
var static_type_pattern: RegEx

var static_type_no_equals_pattern: RegEx
var typed_for_pattern: RegEx

var is_in_multiline_comment: bool = false
var is_in_multiline_doc_comment: bool = false
var is_inside_open_paren: bool = false
var is_compilation_enabled: bool = true

var is_compilation_enabled_stack: Array[bool] = []

var constexpr_variables: Dictionary[String, Variant] = {}
var consteval_variables: Dictionary[String, Variant] = {}

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

	block_comment_pattern = RegEx.new()
	block_comment_pattern.compile(r"/\*.*?\*/")

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

	static_type_no_equals_pattern = RegEx.new()
	static_type_no_equals_pattern.compile(r"{type_pattern}\s+{var_pattern}\n".format({"type_pattern": type_pattern, "var_pattern": var_pattern}))

	typed_for_pattern = RegEx.new()
	typed_for_pattern.compile(r"for\s+{type_pattern}\s+{var_pattern}".format({"type_pattern": type_pattern, "var_pattern": var_pattern}))

func exec_and_return(expression: String) -> Variant: 
	var expr := Expression.new()
	expr.parse(expression)
	return expr.execute()

func write_array(array: PackedStringArray, file: FileAccess) -> void:
	var line: String = " ".join(array)
	file.store_line(line)

func replace_consteval_vars(line_tokens: PackedStringArray) -> PackedStringArray:
	for variable in consteval_variables:
		var iter_idx: int = 0
		for token in line_tokens:
			if token == variable:
				line_tokens[iter_idx] = consteval_variables[variable]
			iter_idx += 1

	return line_tokens

func replace_constexpr_vars(line_tokens: PackedStringArray) -> PackedStringArray:
	for variable in constexpr_variables:
		var iter_idx: int = 0
		for token in line_tokens:
			if token == variable:
				line_tokens[iter_idx] = constexpr_variables[variable]
			iter_idx += 1

	return line_tokens

func get_regex_groups(regex: RegEx, pattern: String) -> PackedStringArray:
	var constexpr_var_match: RegExMatch = regex.search(pattern)
	return constexpr_var_match.strings

func get_indentation_level(line: String) -> int:
	var level: int = 0
	for char in line:
		if char == "\t" or char == "    ":
			level += 1
		else:
			break

	return level

func process_line(line: String, file: FileAccess) -> void:
	var indentation_level: int = get_indentation_level(line)
	line = line.strip_edges()
	var line_tokens: PackedStringArray = whitespace_split(line)
	if (line_tokens.is_empty()):
		file.store_line("")
		return

	var first_token: String = line_tokens[0]
	var last_token: String = line_tokens[line_tokens.size() - 1]

	if first_token == "#else":
		is_compilation_enabled = not is_compilation_enabled
		return

	if first_token == "#endif":
		if is_compilation_enabled_stack.size() > 0:
			is_compilation_enabled = is_compilation_enabled_stack.pop_back()
		else:
			is_compilation_enabled = true

		if is_compilation_enabled_stack.size() <= 0:
			is_compilation_enabled = true

		return

	if not is_compilation_enabled:
		return

	if first_token == "fn":
		line_tokens[0] = "func"
		first_token = "func"

	if (first_token == "static" and line_tokens[1] == "fn"):
		line_tokens[1] = "func"

	# Multi line documentation comments
	if first_token == '"""':
		if not is_in_multiline_doc_comment:
			is_in_multiline_doc_comment = true
		else:
			is_in_multiline_doc_comment = false

		return

	if is_in_multiline_doc_comment:
		file.store_line("## " + line)
		return

	# Ignore comments
	if first_token == "#":
		for token in preprocessor_statements:
			if token == first_token:
				file.store_line(line)
				return

	# Multi line comments
	if first_token == "/*":
		is_in_multiline_comment = true
		line_tokens[0] = "#"

		write_array(line_tokens, file)
		return

	if is_in_multiline_comment and first_token == "*/" or last_token == "*/":
		is_in_multiline_comment = false
		if first_token == "*/":
			line_tokens[0] = "#"
		else:
			line_tokens[line_tokens.size() - 1] = "# "

		write_array(line_tokens, file)
		return

	if not is_in_multiline_comment:
		var line_str: String = " ".join(line_tokens)
		line_str = block_comment_pattern.sub(line_str, "", false)
		line_tokens = whitespace_split(line_str)

		first_token= line_tokens[0]
		last_token= line_tokens[line_tokens.size() - 1]

	# Replace consteval variables with their values
	line_tokens = replace_consteval_vars(line_tokens)

	# Constexpr variables
	if first_token == "constexpr":
		line_tokens = replace_constexpr_vars(line_tokens)

		var line_str: String = " ".join(line_tokens)
		var groups: PackedStringArray = get_regex_groups(constexpr_var_pattern, line_str)
		if groups.size() > 0:
			return

		var var_name: String = groups[2]
		var expression: String = groups[3].strip_edges()
		var expression_result: Variant = exec_and_return(expression)
		constexpr_variables[var_name] = expression_result

		var idx: int = 0
		for token in line_tokens:
			token = token.replace(expression, str(expression_result))
			if token == "constexpr":
				token = "const"
			line_tokens[idx] = token

			idx += 1

	if first_token == "consteval":
		line_tokens = replace_constexpr_vars(line_tokens)

		var line_str: String = " ".join(line_tokens)
		var groups: PackedStringArray = get_regex_groups(consteval_var_pattern, line_str)
		if groups.size() > 0:
			return

		var var_name: String = groups[2]
		var expression: String = groups[3].strip_edges()
		var expression_result: Variant = exec_and_return(expression)
		consteval_variables[var_name] = expression_result
		return

	if first_token == "#if":
		line_tokens = replace_constexpr_vars(line_tokens)
		var pp_tokens: PackedStringArray = line_tokens
		pp_tokens.remove_at(0)

		var expression: String = " ".join(pp_tokens)
		var expression_result: Variant = exec_and_return(expression)
		if (expression_result is bool or expression_result is int):
			is_compilation_enabled = expression_result
			is_compilation_enabled_stack.append(is_compilation_enabled)
		else:
			print("Wrong type for preprocessor statement")

		return

	if last_token == '(':
		is_inside_open_paren = true

	if is_inside_open_paren and (last_token == ')' or last_token == '):'):
		is_inside_open_paren = false

	var line_str = " ".join(line_tokens)
	var comment_string: String = ""
	var line_split: PackedStringArray = line_str.split("#")
	var split_line: String = line_split[0]

	# Make it so you don't need to use colons when the compiler can figure out where one should go
	if (
		first_token == "static func"
		or first_token == "func"
		or first_token == "for"
		or first_token == "while"
		or first_token == "class"
		or first_token == "match"
		or first_token == "else"
	) and not (last_token == ':' or first_token == "class_name" or is_inside_open_paren):
		if line_split.size() > 1:
			comment_string = "#" + line_split[1]
		line_str = split_line.substr(0, split_line.length()) + ": " + comment_string + split_line.substr(split_line.length(), 1)
		line_tokens = whitespace_split(line_str)

	line_split = line_str.split("#")
	split_line = line_split[0]

	if not last_token == "\\" and not is_inside_open_paren and first_token == "if" or first_token == "elif":
		if not " ".join(line_tokens).contains(":"):
			comment_string = ""
			if line_split.size() > 1:
				comment_string = "#" + line_split[1]
			line_str = split_line.substr(0, split_line.length()) + ": " + comment_string + split_line.substr(split_line.length(), 1)
			line_tokens = whitespace_split(line_str)

	line_split = line_str.split("#")
	split_line = line_split[0]

	# Typed function params
	if first_token == "func":
		if line_split.size() > 1:
			comment_string = "#" + line_split[1]

		var no_equals_match: Array[RegExMatch] = static_type_func_no_equals_pattern.search_all(line_str)
		if no_equals_match.size() > 1:
			for match: RegExMatch in no_equals_match:
				var groups: PackedStringArray = match.strings
				if not groups[1].begins_with("func") and not groups[1].begins_with("static"):
					split_line = split_line.replace("%s %s" % [groups[1], groups[2]], "%s: %s" % [groups[2], groups[1]])

			line_str = split_line.substr(0, split_line.length()) + comment_string + split_line.substr(split_line.length(), 1)
			line_tokens = whitespace_split(line_str)

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

	var annotation_match: RegExMatch = annotation_pattern.search(line_str)
	if annotation_match:
		line_str = annotation_pattern.sub(line_str, r"@$1 var $3: $2$4")

	var static_type_var_declaration_match: RegExMatch = static_type_pattern.search(line_str)
	if static_type_var_declaration_match:
		var static_type_var_groups: PackedStringArray = static_type_var_declaration_match.strings
		if (static_type_var_declaration_match and not static_type_var_groups[1] in ["var", "const"]
			and not (first_token == "func" or first_token == "static func")
		):
			line_str = static_type_pattern.sub(line_str, r"var $2: $1 =$3")

	var no_equals_match: RegExMatch = static_type_no_equals_pattern.search(line_str)
	if no_equals_match and not (first_token == "return" or first_token == "extends" or first_token == "class_name" or first_token == "func" or first_token == "static"):
		line_str = static_type_no_equals_pattern.sub(line_str, r"var $2: $1\n")

	if first_token == "const":
		line_str = line_str.replace("const var", "const")

	# Typed for loop
	var typed_for_match: RegExMatch = typed_for_pattern.search(line_str)
	if typed_for_match:
		var typed_for_match_groups: PackedStringArray = typed_for_match.strings
		if typed_for_match and typed_for_match_groups[2] != "in":
			line_str = typed_for_pattern.sub(line_str, r"for $2: $1")

	if is_in_multiline_comment:
		line_str = "# " + line_str

	for level in range(indentation_level):
		line_str = "\t" + line_str
	file.store_line(line_str)


func compile(input_file_name: String, output_file_name: String) -> void: 
	var file := FileAccess.open(input_file_name, FileAccess.READ)
	var out_file := FileAccess.open(output_file_name, FileAccess.WRITE)
	while not file.eof_reached():
		process_line(file.get_line(), out_file)
	file.close()

func _init(): 
	compile_regex()
	compile("test.gdp", "output.gd")
