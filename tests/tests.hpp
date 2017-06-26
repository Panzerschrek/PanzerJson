#pragma once
#include "../src/panzer_json_assert.hpp"

#define test_assert(x) PJ_ASSERT(x)
#define test_assert_near(x,y, eps) test_assert(std::abs((x) - (y)) <= std::abs(eps) )
