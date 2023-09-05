#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#include <ddraw.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <Windows.h>

#include "safe99_common/assert.h"
#include "safe99_common/defines.h"
#include "safe99_common/safe_delete.h"

#include "safe99_math/math_util.h"

#include "renderer_ddraw.h"

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

#endif // PRECOMPILED_H