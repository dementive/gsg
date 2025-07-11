#pragma once
/*
	FOR_EACH recursive macro from this article: https://www.scs.stanford.edu/~dm/blog/va-opt.html

	Example Usage:
	#define F(m_param) 2 * m_param
	FOR_EACH(F, a, b, c, 1, 2, 3)   // => F(a) F(b) F(c) F(1) F(2) F(3)

	Note that this will only work with C++20 or a newer version because of __VA_OPT__
*/

namespace GC {

#define PARENS ()
#define PAIR(first, second) first, second

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

// FOR_EACH with 1 argument but the macro takes 2 arguments
// Example Usage: FOR_EACH_TWO_ARGS(F, X, __VA_OPT__(__VA_ARGS__,))
#define FOR_EACH_TWO_ARGS(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER_TWO_ARGS(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER_TWO_ARGS(macro, a1, a2, ...) macro(a1, a2) __VA_OPT__(FOR_EACH_AGAIN_TWO_ARGS PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN_TWO_ARGS() FOR_EACH_HELPER_TWO_ARGS

// FOR_EACH with 2 arguments but 1 of the arguments is always the same.
// Example Usage: FOR_EACH_TWO(F, X, __VA_OPT__(__VA_ARGS__,))
#define FOR_EACH_TWO(macro, a1, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER_TWO(macro, a1, __VA_ARGS__)))
#define FOR_EACH_HELPER_TWO(macro, a1, a2, ...) macro(a1, a2) __VA_OPT__(FOR_EACH_AGAIN_TWO PARENS(macro, a1, __VA_ARGS__))
#define FOR_EACH_AGAIN_TWO() FOR_EACH_HELPER_TWO

// FOR_EACH with 3 arguments but 1 of the arguments is always the same.
#define FOR_EACH_THREE(macro, a1, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER_THREE(macro, a1, __VA_ARGS__)))
#define FOR_EACH_HELPER_THREE(macro, a1, a2, a3, ...) macro(a1, a2, a3) __VA_OPT__(FOR_EACH_AGAIN_THREE PARENS(macro, a1, __VA_ARGS__))
#define FOR_EACH_AGAIN_THREE() FOR_EACH_HELPER_THREE

} // namespace GC
