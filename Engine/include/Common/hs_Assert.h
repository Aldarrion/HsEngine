#pragma once
#include "Config.h"

#include <cassert>

#define HS_ASSERT(x) assert(x)

#define HS_NOT_IMPLEMENTED HS_ASSERT(!"Not implemented");
