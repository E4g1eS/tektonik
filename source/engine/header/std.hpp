#ifndef STD_HPP
#define STD_HPP

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <concepts>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

// Macros that cannot be replaced by fancy C++ constructs.
// Mostly conditional compilation that can actually completely remove code.

// Asserts in debug mode, assumes in release mode.
#ifdef NDEBUG
#define ASSUMERT(condition) [[assume(condition)]]
#else
#define ASSUMERT(condition) assert(condition)
#endif

#ifdef NDEBUG
#define DEBUG_ONLY(code)
#else
#define DEBUG_ONLY(code) code
#endif

#endif
