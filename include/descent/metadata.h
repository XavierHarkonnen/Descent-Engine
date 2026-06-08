/* Copyright 2025 XavierHarkonnen9 and Enlarium
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DESCENT_METADATA_H
#define DESCENT_METADATA_H

#include <descent/build.h>
#include <descent/types.h>

#define ENGINE_NAME "Descent Engine"

#define ENGINE_VERSION_VARIANT 0
#define ENGINE_VERSION_MAJOR   0
#define ENGINE_VERSION_MINOR   1
#define ENGINE_VERSION_PATCH   0

#define MAKE_VERSION(variant, major, minor, patch) \
	((((u32)(variant)) << 29U) | (((u32)(major)) << 22U) | (((u32)(minor)) << 12U) | ((u32)(patch)))

#define ENGINE_VERSION MAKE_VERSION(ENGINE_VERSION_VARIANT, ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH)

_Static_assert(ENGINE_VERSION_VARIANT <= 7,    "BUILD_VERSION_VARIANT must not exceed 7 (3 bits)");
_Static_assert(ENGINE_VERSION_MAJOR   <= 31,   "BUILD_VERSION_MAJOR must not exceed 31 (5 bits)");
_Static_assert(ENGINE_VERSION_MINOR   <= 1023, "BUILD_VERSION_MINOR must not exceed 1023 (10 bits)");
_Static_assert(ENGINE_VERSION_PATCH   <= 4095, "BUILD_VERSION_PATCH must not exceed 4095 (12 bits)");

#endif