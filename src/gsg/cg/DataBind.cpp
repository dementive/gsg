#include "DataBind.hpp"

#include "scene/gui/control.h"

#include "cg/Utility.hpp"

using namespace CG;

DataBind *DataBind::init(const String &p_path) {
	Node *scene = UT::init_scene(p_path);
	DataBind *databind = Object::cast_to<DataBind>(scene->get_child(0));

	if (databind == nullptr) [[unlikely]]
		return nullptr;
	;

	databind->init_databind();
	return databind;
}

void DataBind::init_databind() {
	_find_metadata_properties(get_parent());
	set_process(true);
}

Ref<Expression> DataBind::get_expression(const String &expression_string) {
	const Ref<Expression> expression = memnew(Expression());
	const Error err = expression->parse(expression_string);
	if (err != OK)
		print_error(expression->get_error_text());

	return expression;
}

const Array dummy_input_array; // empty array must be passed into Expression::execute and it would be stupid to construct a new one every time.

template <typename T> void DataBind::execute(const T &callable, Control *node, const StringName &method, Variant::Type expected_type, const StringName &expected_class) {
	// If the method to call doesn't exist there is no reason to even execute the expression
#ifdef DEBUG_ENABLED
	if (!node->has_method(method)) [[unlikely]] {
		print_error(String("Executing " + method + " expression for " + String(node->get_path()) + " failed: " + method + " does not exist."));
		return;
	}
#endif

	Variant result;

	// Check if T is an Expression or Callable and get result
	if constexpr (std::is_same_v<std::decay_t<T>, Ref<Expression>>) {
		result = callable->execute(dummy_input_array, base_instance);

		// Bail if execution fails
		if (callable->has_execute_failed()) [[unlikely]] {
			print_error(String("Executing " + method + " expression for " + String(node->get_path()) + " failed: " + callable->get_error_text()));
			return;
		}
	} else if constexpr (std::is_same_v<std::decay_t<T>, MethodBind *>) {
		Callable::CallError call_error;
		result = callable->call(base_instance, nullptr, 0, call_error);
	}

#ifdef DEBUG_ENABLED

	// If variant types don't match return
	if (result.get_type() != expected_type) [[unlikely]] {
		if ((expected_type == Variant::STRING && result.get_type() == Variant::INT)) { // allow int if expected type is String.
			result = result.stringify();
		} else {
			print_error(String("Executing " + method + " expression for " + String(node->get_path()) + " failed: Result type is " + Variant::get_type_name(result.get_type()) +
					" expected: " + Variant::get_type_name(expected_type)));
			return;
		}
	}

	// If type is Object also verify that the class is correct.
	if (!expected_class.is_empty() and result.get_type() == Variant::OBJECT) {
		Object *obj = Object::cast_to<Object>(result); // need to cast to call is_class
		if (obj == nullptr) [[unlikely]] // This is fucking impossible but happens anyway sometimes
			return;
		if (!obj->is_class(expected_class)) [[unlikely]] {
			print_error(String("Executing " + method + " expression for " + String(node->get_path()) + " failed: Result class is " + Variant::get_type_name(result.get_type()) +
					" expected: " + expected_class));
			return;
		}
	}

#endif
	// Call the godot method with the result of the expression
	// For example if the metadata is 'visible', this will call the set_visible method.
	node->call(method, result);
}

void DataBind::setup_pressed(Control *node) {
	if (node->has_meta("pressed") and node->is_class("BaseButton")) {
		const String &pressed_method = node->get_meta("pressed");
		Callable pressed_callable = Callable(base_instance, pressed_method);

		if (!pressed_callable.is_valid()) {
			// If callable has arguments try making it an Expression
			const Ref<Expression> expression = get_expression(pressed_method);
			pressed_expressions.push_back(expression); // need to take ownership or connect won't work
			pressed_callable = callable_mp(*expression, &Expression::execute).bind(dummy_input_array, base_instance, true, false);

			if (!pressed_callable.is_valid()) {
				print_error("Callable '" + pressed_method + "' assigned to 'pressed' signal for " + node->get_name() + " is not valid.");
				return;
			}
		}
		node->connect("pressed", pressed_callable);
	}
}

void DataBind::setup_datamodel(Control *node) {
	if (!node->has_meta("datamodel"))
		return;

	const Callable callable = Callable(base_instance, node->get_meta("datamodel"));
	const Array result = callable.call();
	for (const Variant &var : result) {
		Node *item = Object::cast_to<Node>(var);
		if (item == nullptr)
			continue;

		node->add_child(item);
	}
}

#define NEW_SET_PROPERTY(m_property, m_type)                                                                                                                                                 \
	if (node->has_meta(m_property)) {                                                                                                                                                        \
		MethodBind *method = ClassDB::get_method(base_instance->get_class_name(), node->get_meta(m_property));                                                                               \
                                                                                                                                                                                             \
		if (method != nullptr) {                                                                                                                                                             \
			const DataBindCallableProperty property{ .property_type = m_type, .callable = method };                                                                                                \
			data_bind_node.callable_properties.push_back(property);                                                                                                                          \
		} else {                                                                                                                                                                             \
			const DataBindExpressionProperty property{ .property_type = m_type, .callable = get_expression(node->get_meta(m_property)) };                                                          \
			data_bind_node.expression_properties.push_back(property);                                                                                                                        \
		}                                                                                                                                                                                    \
	}

void DataBind::_find_metadata_properties(Node *node_to_check) { // NOLINT(misc-no-recursion)
	const TypedArray<Node> children = node_to_check->get_children(false);
	for (const Variant &child : children) {
		Control *node = Object::cast_to<Control>(child);
		if (node == nullptr)
			continue;

		// IMPORTANT - the DataBind Node should always be the first child of the GUI scene root node. It can't be the root because it breaks Container layouts in nested scenes...
		if (node->get_owner()->get_child(0) != this)
			continue;

		if (node->get_child_count() > 0)
			_find_metadata_properties(node);

		DataBindNode data_bind_node;

		NEW_SET_PROPERTY("visible", VISIBLE)
		NEW_SET_PROPERTY("disabled", DISABLED)
		NEW_SET_PROPERTY("text", TEXT)
		NEW_SET_PROPERTY("texture", TEXTURE)
		NEW_SET_PROPERTY("icon", ICON)
		NEW_SET_PROPERTY("tooltip", TOOLTIP)
		NEW_SET_PROPERTY("progress", PROGRESS)

		setup_pressed(node);
		setup_datamodel(node);

		if (data_bind_node.expression_properties.size() > 0 or data_bind_node.callable_properties.size() > 0) {
			data_bind_node.node = node;
			nodes.push_back(data_bind_node);
		}
	}
}

void DataBind::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			update();
		} break;
	}
}

_ALWAYS_INLINE_ void DataBind::update_properties(Control *node, const auto &property) {
	// Have to run visible property every update no matter what, for all other properties only update if the Control is visible.
	if (property.property_type != VISIBLE and !node->is_visible_in_tree())
		return;

	switch (property.property_type) {
		case VISIBLE: {
			execute(property.callable, node, "set_visible", Variant::BOOL);
		} break;
		case DISABLED: {
			execute(property.callable, node, "set_disabled", Variant::BOOL);
		} break;
		case TEXT: {
			execute(property.callable, node, "set_text", Variant::STRING);
		} break;
		case TEXTURE: {
			execute(property.callable, node, "set_texture", Variant::OBJECT, "Texture2D");
		} break;
		case ICON: {
			execute(property.callable, node, "set_button_icon", Variant::OBJECT, "Texture2D");
		} break;
		case TOOLTIP: {
			execute(property.callable, node, "set_tooltip_text", Variant::STRING);
		} break;
		case PROGRESS: {
			execute(property.callable, node, "set_value_no_signal", Variant::FLOAT);
		} break;
	}
}

void DataBind::update() {
	for (const auto &data_bind_node : nodes) {
		for (const auto &property : data_bind_node.callable_properties)
			update_properties(data_bind_node.node, property);
		for (const auto &property : data_bind_node.expression_properties)
			update_properties(data_bind_node.node, property);
	}
}

void DataBind::set_base_instance(Object *p_object) { base_instance = p_object; }

void DataBind::_bind_methods() {}
