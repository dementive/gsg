#pragma once

#include "core/math/expression.h"

#include "scene/main/node.h"

class Control;

#define create_databind(m_class, m_scene) Object::cast_to<m_class>(DataBind::init(m_scene))

namespace CG {

class DataBind : public Node {
	GDCLASS(DataBind, Node)

private:
	enum DataBindProperty : uint8_t {
		VISIBLE,
		DISABLED,
		TEXT,
		TEXTURE,
		ICON,
		TOOLTIP,
		PROGRESS,
	};

	struct DataBindExpressionProperty {
		DataBindProperty property_type{};
		Ref<Expression> callable;
	};

	struct DataBindCallableProperty {
		DataBindProperty property_type{};
		MethodBind *callable{};
	};

	struct DataBindNode {
		Control *node{};
		TightLocalVector<DataBindExpressionProperty> expression_properties;
		TightLocalVector<DataBindCallableProperty> callable_properties;
	};

	TightLocalVector<DataBindNode> nodes;
	TightLocalVector<Ref<Expression>> pressed_expressions;
	Object *base_instance{};

	static Ref<Expression> get_expression(const String &expression_string);
	void setup_pressed(Control *node);
	void setup_datamodel(Control *node);

	template <typename T> void execute(const T &callable_or_expr, Control *node, const StringName &method, Variant::Type expected_type, const StringName &expected_class = "");
	void update_properties(Control *node, const auto &property);

	// Fill node_expressions with all nodes that are Controls, have ceratin metadata properties, and are owned by this->parent.
	void _find_metadata_properties(Node *node_to_check);
	void init_databind();

	// Execute all DataBind metadata properties and then update the UI with the result of each one.
	void update();

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void set_base_instance(Object *p_object);

public:
	// Call to init DataBind scene.
	// Loads scene file from disc and then fills all DataBind metadata properties.
	static DataBind *init(const String &p_path);
};

} // namespace CG
