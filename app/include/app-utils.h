/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_utils_h_
#define __app_utils_h_

#include <zephyr.h>

#define NELEMENTS(x)    ((sizeof((x)) / sizeof((x)[0])))

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define clamp(x, y, z) min(max((x), (y)), (z))


#endif /* __app_utils_h_ */