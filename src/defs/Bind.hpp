#pragma once

namespace CG {

// Macros to make binding nodes to godot easier
// Most checks can be skipped in a release build and are only needed when developing the game with the editor so there are multiple versions of these.

#ifdef TOOLS_ENABLED

// Define a property's setter and getter functions in a header file.
#define PROP(m_property_name, m_type)                                                                                                                                                        \
	void set_##m_property_name(const m_type &p_##m_property_name) {                                                                                                                          \
		if (is_class("Node")) {                                                                                                                                                              \
			call("update_configuration_warnings");                                                                                                                                           \
		}                                                                                                                                                                                    \
		m_property_name = p_##m_property_name;                                                                                                                                               \
	}                                                                                                                                                                                        \
	m_type get_##m_property_name() const { return m_property_name; }

// Same as PROP but for a pointer since pointers can't be const &
#define PROP_PTR(m_property_name, m_type)                                                                                                                                                    \
	void set_##m_property_name(m_type *p_##m_property_name) {                                                                                                                                \
		if (is_class("Node")) {                                                                                                                                                              \
			call("update_configuration_warnings");                                                                                                                                           \
		}                                                                                                                                                                                    \
		m_property_name = p_##m_property_name;                                                                                                                                               \
	}                                                                                                                                                                                        \
	m_type *get_##m_property_name() const { return m_property_name; }

// Similar to PROP but only for use when binding an OBJECT pointer to a Node.
#define GUI_NODE(m_name, m_class)                                                                                                                                                            \
private:                                                                                                                                                                                     \
	m_class *m_name = nullptr;                                                                                                                                                               \
                                                                                                                                                                                             \
public:                                                                                                                                                                                      \
	m_class *get_##m_name() const { return m_name; }                                                                                                                                         \
	void set_##m_name(m_class *new_##m_name) {                                                                                                                                               \
		m_name = new_##m_name;                                                                                                                                                               \
		update_configuration_warnings();                                                                                                                                                     \
	}

// Connect to a button's pressed signal, note the ':on_##m_name##_pressed' naming convention for callback functions.
#define CONNECT_CALLBACK(m_class, m_name)                                                                                                                                                    \
	if (m_name != nullptr)                                                                                                                                                                   \
		m_name->connect("pressed", callable_mp(this, &m_class::on_##m_name##_pressed));

// Connect to a signal's on_X signal/method.
#define CONNECT(m_class, m_signal, m_name)                                                                                                                                                   \
	if (m_name != nullptr)                                                                                                                                                                   \
		m_name->connect(#m_signal, callable_mp(this, &m_class::on_##m_name##_##m_signal));

// Connect to a signal's static on_X signal/method.
#define CONNECT_STATIC(m_class, m_signal, m_name)                                                                                                                                            \
	if (m_name != nullptr)                                                                                                                                                                   \
		m_name->connect(#m_signal, callable_mp_static(&m_class::on_##m_name##_##m_signal));

#else

// Same as above but no safety checks or configuration warnings for release builds.
#define PROP(m_property_name, m_type)                                                                                                                                                        \
	void set_##m_property_name(const m_type &p_##m_property_name) { m_property_name = p_##m_property_name; }                                                                                 \
	m_type get_##m_property_name() const { return m_property_name; }

#define PROP_PTR(m_property_name, m_type)                                                                                                                                                    \
	void set_##m_property_name(m_type *p_##m_property_name) { m_property_name = p_##m_property_name; }                                                                                       \
	m_type *get_##m_property_name() const { return m_property_name; }

#define GUI_NODE(m_name, m_class)                                                                                                                                                            \
	m_class *m_name = nullptr;                                                                                                                                                               \
	m_class *get_##m_name() const { return m_name; }                                                                                                                                         \
	void set_##m_name(m_class *new_##m_name) { m_name = new_##m_name; }

#define CONNECT_CALLBACK(m_class, m_name) m_name->connect("pressed", callable_mp(this, &m_class::on_##m_name##_pressed));

#define CONNECT(m_class, m_signal, m_name) m_name->connect(#m_signal, callable_mp(this, &m_class::on_##m_name##_##m_signal));

#define CONNECT_STATIC(m_class, m_signal, m_name) m_name->connect(#m_signal, callable_mp_static(&m_class::on_##m_name##_##m_signal));

#endif // TOOLS_ENABLED

// Bind a Node pointer in _bind_methods
#define BIND_NODE(m_type, m_name, m_hint_type)                                                                                                                                               \
	ClassDB::bind_method(D_METHOD("get_" #m_name), &m_type::get_##m_name);                                                                                                                   \
	ClassDB::bind_method(D_METHOD("set_" #m_name, "new_" #m_name), &m_type::set_##m_name);                                                                                                   \
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, #m_name, PROPERTY_HINT_NODE_TYPE, #m_hint_type), "set_" #m_name, "get_" #m_name);

// Bind a simple property without a hint in _bind_methods
#define BIND_PROP(m_type, m_name, m_variant_type)                                                                                                                                            \
	ClassDB::bind_method(D_METHOD("get_" #m_name), &m_type::get_##m_name);                                                                                                                   \
	ClassDB::bind_method(D_METHOD("set_" #m_name, #m_name), &m_type::set_##m_name);                                                                                                          \
	ADD_PROPERTY(PropertyInfo(Variant::m_variant_type, #m_name), "set_" #m_name, "get_" #m_name);

// for use with last argument of BIND_PROP_HINT
#define HINT(first, second) first, second

// Bind a property with a hint in _bind_methods
#define BIND_PROP_HINT(m_type, m_name, m_variant_type, m_hint)                                                                                                                               \
	ClassDB::bind_method(D_METHOD("get_" #m_name), &m_type::get_##m_name);                                                                                                                   \
	ClassDB::bind_method(D_METHOD("set_" #m_name, #m_name), &m_type::set_##m_name);                                                                                                          \
	ADD_PROPERTY(PropertyInfo(Variant::m_variant_type, #m_name, m_hint), "set_" #m_name, "get_" #m_name);

// Call CHECK_NODE in _get_configuration_warnings to report misconfigured nodes in editor.
#define CHECK_NODE(m_name)                                                                                                                                                                   \
	if (m_name == nullptr)                                                                                                                                                                   \
		warnings.append(vformat("%s node pointer is not set!", #m_name));

} // namespace CG
