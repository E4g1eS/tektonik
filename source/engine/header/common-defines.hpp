#ifndef STD_HPP
#define STD_HPP

// Macros that cannot be replaced by fancy C++ constructs.
// Mostly conditional compilation that can actually completely remove code.

// Asserts in debug mode, assumes in release mode.
#ifdef NDEBUG
#define ASSUMERT(condition) [[assume(condition)]]
#else
#define ASSUMERT(condition) //assert(condition)
#endif

#ifdef NDEBUG
#define DEBUG_ONLY(code)
#else
#define DEBUG_ONLY(code) code
#endif

#ifdef NDEBUG
#define DONT_COMPILE_TESTS
#endif

#endif
