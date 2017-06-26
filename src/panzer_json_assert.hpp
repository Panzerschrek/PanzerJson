#pragma once

// if you wish to use cutom assert handler for this library,
// you may define PANZER_JSON_CUSTOM_ASSERT_HANDLER macro, create file with
// cutom assert handler and add path to this file to includes directory of this
// library.
#ifdef PANZER_JSON_CUSTOM_ASSERT_HANDLER

#include <panzer_json_custom_assert_handler.hpp>
#ifndef PJ_ASSERT
#error "PJ_ASSERT must be defined in file with custom assert handler."
#endif

#else

// Enable asserts only if DEBUG macro enabled
#ifndef DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif

#include <cassert>
#define PJ_ASSERT(x) assert(x)

#endif
