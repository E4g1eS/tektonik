export module common;
// This module should have no dependencies.

namespace tektonik::common
{

#ifdef NDEBUG
export constexpr bool kDebugBuild = false;
#else
export constexpr bool kDebugBuild = true;
#endif

}
