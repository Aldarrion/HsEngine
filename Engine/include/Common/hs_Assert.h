#pragma once
#include "Config.h"

#include <cassert>

#define hs_assert(x) assert(x)

#define HS_NOT_IMPLEMENTED hs_assert(!"Not implemented");
