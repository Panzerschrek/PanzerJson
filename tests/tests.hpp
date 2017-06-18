#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#define test_assert(x) assert(x)
#define test_assert_near(x,y, eps) test_assert(std::abs((x) - (y)) <= std::abs(eps) )
