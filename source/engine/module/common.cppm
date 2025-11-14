export module common;
// This module should have no dependencies other than std.

import std;

namespace tektonik
{

// Types
export {
    using u32 = std::uint32_t;
    using i32 = std::int32_t;
}

namespace common
{

#ifdef NDEBUG
export constexpr bool kDebugBuild = false;
#else
export constexpr bool kDebugBuild = true;
#endif

}

}
