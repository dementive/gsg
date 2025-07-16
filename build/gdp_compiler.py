import re

"""
Gdscript preprocessor

Does a single pass that just replaces text with other text. This adds nothing but syntax sugar, it does not add any new features to gdscript that do not already exist.

Features:

- Some hardcoded macros to make typing some keywords easier. See keywords_map for the full list of keywords.
    Ex: 'fn' instead of 'func'.
    Note that these act like keywords so you cannot use these words in your code as variable or function names, they are reserved for the compiler.

- Improved static typing syntax that looks more like C typing.
    For example instead of doing `var test_var: int = 999` you can now do `int test_var = 999`.
    This will work for all types, even user defined ones and container types like Array[String].

- Using ':' at the end of most statements is now optional instead of required.
    This includes the keywords if/elif/else, func, class, while, for, and match. Multiline statements will still require a colon.

- Multi line comments with /* and */. 
    For this to work start a line with /*. All other lines until another line that only has */ is found will be commented out.

- Multi line documentation comments using triple quotes just like how they work in python.

- In block comments using /* and */ work.
    This allows you to comment out only a part of a line, which isn't possible with just gdscript.
    Example: `int test_doc_comment_var /* = 999 */ = 500` would compile to `var test_doc_comment_var: int = 500`

- Easier to write `@export` and  `@onready` notation
    Example: `export String string_export` gets compiled to `@export var string_export: String`
             `onready int ready_int_number = 5` gets compiled to `@onready var ready_int_number: int = 5`

- Adds a keyword: "new" that can be used to write `new Object` instead of `Object.new()`. This works for any object type that .new() gets called on.

- Adds a keyword: "free" that can be used to write `free Object` instead of `Object.free()`. This works for any object type that .free() gets called on.

- Adds a keyword: "qfree" that can be used to write `qfree node` instead of `node.queue_free()`. This works for any node type that .queue_free() gets called on.

IMPORTANT: The preprocessor isn't a proper compiler, it is entirely based on vibes.

It doesn't do hardly any error checking but you can open the resulting compiled gdscript file in the editor to see if there are any errors.
There may also be bugs with the output if the compiler determines the vibes in your script aren't right.
There is a lot of gdscript syntax and getting 100% of it to transpile without any edge cases is tough.

"""

keywords_map = {
    "yes": "true",
    "no": "false",
    "fn": "func",
    "vec2": "Vector2",
    "vec3": "Vector3",
    "vec4": "Vector4",
    "ivec2": "Vector2i",
    "ivec3": "Vector3i",
    "ivec4": "Vector4i",
    "Map": "Dictionary",
}


def gds_compile(input_filename, output_filename):
    var_pattern = r"([A-Za-z_0-9]\w*)"
    type_pattern = r"([A-Za-z_0-9]\w*(?:\[[^\[\]]+\])?)"

    memnew_pattern = re.compile(rf"new\s+{var_pattern}")
    memfree_pattern = re.compile(rf"free\s+{var_pattern}")
    memqueue_free_pattern = re.compile(rf"qfree\s+{var_pattern}")
    static_type_pattern = re.compile(rf"{type_pattern}\s+{var_pattern}\s+=(.*)")

    static_type_no_equals_pattern = re.compile(rf"{type_pattern}\s+{var_pattern}\n")
    annotation_pattern = re.compile(rf"^(export|onready)\s+{type_pattern}\s+{var_pattern}(.*)")

    block_comment_pattern = re.compile(r"/\*.*?\*/", flags=re.DOTALL)

    is_in_multiline_comment = False
    is_in_multiline_doc_comment = False

    with open(input_filename, "r") as infile, open(output_filename, "w") as outfile:
        for line in infile:
            # Macros that I think are nice
            for keyword in keywords_map:
                line = re.sub(rf"\b{keyword}\b", keywords_map[keyword], line)

            stripped_line = line.strip()

            # Multi line documentation comments
            if stripped_line.startswith('"""'):
                if not is_in_multiline_doc_comment:
                    is_in_multiline_doc_comment = True
                else:
                    is_in_multiline_doc_comment = False

                continue

            if is_in_multiline_doc_comment:
                outfile.write("## " + line)
                continue

            # Ignore empty
            if not stripped_line:
                outfile.write(line)
                continue

            # Ignore comments
            if stripped_line.startswith("#"):
                outfile.write(line)
                continue

            # Multi line comments
            if stripped_line.startswith("/*"):
                is_in_multiline_comment = True
                line = line.replace("/*", "#")
                stripped_comment_line = line.strip()
                if stripped_comment_line == "#":
                    continue  # continue if no other words on the line but the comment character
                else:
                    outfile.write(line)
                    continue

            if is_in_multiline_comment and stripped_line.startswith("*/") or stripped_line.endswith("*/"):
                is_in_multiline_comment = False
                if stripped_line.startswith("*/"):
                    line = line.replace("*/", "#")
                else:
                    line = "# " + line.replace("*/", "")
                stripped_comment_line = line.strip()

                if stripped_comment_line == "#":
                    continue  # continue if no other words on the line but the comment character
                else:
                    outfile.write(line)
                    continue

            # Inline block comments
            if not is_in_multiline_comment and stripped_line.__contains__("*/"):
                line = re.sub(block_comment_pattern, "", line)

            ends_with_colon = stripped_line.endswith(":")
            starts_with_class_name = stripped_line.startswith("class_name")

            # Make it so you don't need to use colons when the compiler can figure out where one should go
            if (
                stripped_line.startswith("static func")
                or stripped_line.startswith("func")
                or stripped_line.startswith("for")
                or stripped_line.startswith("while")
                or stripped_line.startswith("class")
                or stripped_line.startswith("match")
                or stripped_line.startswith("else")
            ) and not (ends_with_colon or starts_with_class_name):
                line = line.rstrip() + ":\n"

            if (line.endswith("\n") and not stripped_line.endswith("\\")) and stripped_line.startswith("if") or stripped_line.startswith("elif"):
                if not stripped_line.__contains__(":"):
                    line = line.rstrip() + ":\n"

            line = memnew_pattern.sub(r"\1.new()", line)  # Allow using "new Object" syntax in place of "Object.new()"
            line = memqueue_free_pattern.sub(r"\1.queue_free()", line)
            line = memfree_pattern.sub(r"\1.free()", line)

            # onready and export annotations
            annotation_match = re.search(annotation_pattern, line)
            if annotation_match:
                line = annotation_pattern.sub(r"@\1 var \3: \2\4", line)

            # Variable declarations
            static_type_var_declaration_match = re.search(static_type_pattern, line)
            if static_type_var_declaration_match and not static_type_var_declaration_match.group(1) in ["var", "const"]:
                line = static_type_pattern.sub(r"var \2: \1 =\3", line)

            # Variable definitions
            no_equals_match = re.search(static_type_no_equals_pattern, line)
            if no_equals_match and not (stripped_line.startswith("return") or stripped_line.startswith("extends") or stripped_line.startswith("class_name")):
                line = static_type_no_equals_pattern.sub(r"var \2: \1\n", line)

            if line.startswith("const"):
                line = line.replace("const var", "const")

            if is_in_multiline_comment:
                line = "# " + line
            outfile.write(line)


gds_compile("test.gdp", "output.gd")
