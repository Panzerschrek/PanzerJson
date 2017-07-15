#pragma once
#include <iostream>

#define test_assert(x) \
	if(!(x))\
	{\
		std::cout << "Test failed in\n" << __FILE__ << ":" << __LINE__ << "\ncondition \"" << #x << "\" is false" << std::endl;\
		std::exit(-1);\
	}

#define test_assert_near(x,y, eps) test_assert(std::abs((x) - (y)) <= std::abs(eps) )
