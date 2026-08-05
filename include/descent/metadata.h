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

#include <descent/utils.h>

#define ENGINE_NAME "Descent Engine"

#define ENGINE_VERSION_VARIANT 0
#define ENGINE_VERSION_MAJOR   0
#define ENGINE_VERSION_MINOR   1
#define ENGINE_VERSION_PATCH   0

#define ENGINE_VERSION MAKE_VERSION(ENGINE_VERSION_VARIANT, ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH)

_Static_assert(ENGINE_VERSION_VARIANT <= 7,    "ENGINE_VERSION_VARIANT must not exceed 7 (3 bits)");
_Static_assert(ENGINE_VERSION_MAJOR   <= 127,  "ENGINE_VERSION_MAJOR must not exceed 127 (7 bits)");
_Static_assert(ENGINE_VERSION_MINOR   <= 1023, "ENGINE_VERSION_MINOR must not exceed 1023 (10 bits)");
_Static_assert(ENGINE_VERSION_PATCH   <= 4095, "ENGINE_VERSION_PATCH must not exceed 4095 (12 bits)");

#if ENGINE_VERSION_VARIANT == 0
#define ENGINE_ID "descent-engine-" STRINGIFY(ENGINE_VERSION_MAJOR) "." STRINGIFY(ENGINE_VERSION_MINOR) "." STRINGIFY(ENGINE_VERSION_PATCH)
#else
#define ENGINE_ID "descent-engine-" STRINGIFY(ENGINE_VERSION_MAJOR) "." STRINGIFY(ENGINE_VERSION_MINOR) "." STRINGIFY(ENGINE_VERSION_PATCH) "-v" STRINGIFY(ENGINE_VERSION_VARIANT)
#endif

#endif