@icon("res://path/to/optional/icon.svg")

# (optional) class definition:
class_name MyClass

# Inheritance:
extends BaseClass



func global_function_test() -> void
	print("Hello World!")


@export var int_number: int = 5
@export var string_export: String

@onready var ready_int_number: int = 5
@onready var health_bar: ProgressBar = get_node("UI/LifeBar")

# Member variables.
var a = 5
var s = "Hello"
var arr = [1, 2, 3]
var dict = {"key": "value", 2: 3}

var other_dict: Dictionary[String, int] = {key = "value": other_key = 2}

var typed_var: int
var static_typed_var: int
var new_typed_var: int = 0

var typed_var2: int
var inferred_type := "String"

static var static_typed_var_2: int
static var typed_var2_2: int
static var new_typed_var_2: int = 0
static var inferred_type_2 := "String"

const new_typed_var_23: int = 0
static const var new_typed_var_234: int = 0

const constexpr_test: float = 1.4142135623730951
const constexpr_test2: float = 2.8284271247461903

func constexpr_usage_test_func(x: int) -> void:: 
	var value: int = constexpr_test2 + x
	return value


func consteval_usage_test_func(x: int) -> void:: 
	var value: int = 1.4142135623730951 + x
	return value

const constexpr_test_str: String = "Square root of 8: 2.8284271247461903\nSquare root of 12: 3.4641016151377544"

const TWO_PI: float = 1.5707963267948966

# Conditional compilation with constant expressions

func super_secret_debug_function() -> void
	print("I like cats")

const cond_comp_var: int = 0
const cond_comp_var_two: int = 4

func get_favorite_animal() -> void
	return "Frogs are my favorite"

# Constants.
const ANSWER: int = 42
const THE_NAME = "Charly"
const ANSWER_2: int = 999

# Enums.
enum {UNIT_NEUTRAL, UNIT_ENEMY, UNIT_ALLY}
enum Named {THING_1, THING_2, ANOTHER_THING = -1}

# multiline comments?!
# Built-in vector types.
# var v2 := Vector2(1, 2)
# var v3 := Vector3(1, 2, 3)
# var v4 := Vector4(1, 2, 3, 4)
# they work here too? 

# 	Whoa
# 	There

## The description of the variable. Using multi line doc comments works!
## 
## More words.
## 
## @deprecated: Use [member other_var] instead.
var test_doc_comment_var: int = 500

func some_function(param1: int, param2: int, param3: int) -> int: 
	const local_const = 5
	var is_thing_enabled: bool = false

	if param1 < local_const: # Don't need a : at the end of the line.
 		print(param1)
	elif param2 > 5: 
		print(param2)
	else: 
		print("Fail!")

	for i in range(20): 
		print(i)

	for i: int in range(10): 
		print(i)

	# Single and multiline if statements need a ":"
	if 1 > 0: return

	if param1 < local_const \
		1 > 0:
			print(param1)

	next_state = "idle" if true != false else "fall"

	var angle_degrees: int = 135
	var quadrant: String = (
			"northeast" if angle_degrees <= 90
			else "southeast" if angle_degrees <= 180
			else "southwest" if angle_degrees <= 270
			else "northwest"
	)

	var position := Vector2(250, 350)
	if (
			position.x > 200 and position.x < 400
			and position.y > 300 and position.y < 400
	):
		pass

	while param2 != 0: 
		param2 -= 1

	match param3: 
		3:
			print("param3 is 3!")
		_:
			print("param3 is not 3!")

	var local_var: int = param1 + 3
	return local_var


func something(p1: int, p2: int): 
	super(p1, p2)


func other_something(p1: String = "Hello", p2: int = 50): 
	super.something(p1, p2)

func test_thing(p_thing: int = 1, vec_thing := Vector2(0)) -> float: 
	return 1.0

static func test_static() -> void
	pass

class Item: 
	var a: int = 10

func add(reference: Item, amount: int) -> Item: 
	reference.add(amount)
	return reference

var vehicles: Array[Node] = [$Car, $Plane]
var items: Array[Item] = [Item.new()]
var array_of_arrays: Array[Array] = [[], []]

# Constructor
func _init()
	print("Constructed!")
	var lv = Something.new()
	print(lv.a)

	var custom_node: Node = Node.new()
	custom_node.queue_free()

	var obj: Object = Object.new()
	obj.free()