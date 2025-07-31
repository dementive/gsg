#pragma once
#ifdef SFT_TESTS_ENABLED

#include "gui.hpp"
#include "map.hpp"

namespace CG {

test(game) {
	test_gui();
	test_map();
}

}

#endif
