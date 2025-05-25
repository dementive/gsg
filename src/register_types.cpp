#include "register_types.h"
#include "core/string/print_string.h"
#include "nodes/Map3D.hpp"
#include "defs/soa.hpp"

using namespace CG;

struct SOATest {
	struct InnerStruct { // members that are commonly or always used together can be grouped together with another struct.
		float z;
		float w;
	};
	SOA(
		int, x,
		float, y,
		InnerStruct, inner
	)
};

struct FixedSOATest {
	FixedSizeSOA(
		FixedSOATest, 2,
		int, x,
		float, y
	)
};

void initialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	GDREGISTER_RUNTIME_CLASS(Map3D)

	SOATest soa_test{};
	soa_test.resize(100);
	for (int i = 0; i < 100; ++i) {
		soa_test.set_x(i, i);
		soa_test.set_y(i, i);

		SOATest::InnerStruct inner;
		inner.z=i;
		inner.w=i;
		soa_test.set_inner(i, inner);
	}

	print_line("50th elements in soa struct: ", soa_test.get_x(50), soa_test.get_y(50));

	FixedSOATest fixed_soa_test{};
	fixed_soa_test.init(100);
	for (int i = 0; i < 100; ++i) {
		fixed_soa_test.set_x(i, i);
		fixed_soa_test.set_y(i, i);
	}
	print_line("50th elements in fixed soa struct: ", soa_test.get_x(50), soa_test.get_y(50));
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;
}
