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

#ifndef DESCENT_UTILS_HPP
#define DESCENT_UTILS_HPP

#include <descent/type.hpp>

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

#define CONCAT_INTERNAL(a, b) a##b
#define CONCAT(a, b) CONCAT_INTERNAL(a, b)

#define MAKE_VERSION(variant, major, minor, patch) \
	((((u32)(variant) & 0x7u) << 29u) | (((u32)(major) & 0x7Fu) << 22u) | (((u32)(minor) & 0x3FFu) << 12u) | ((u32)(patch) & 0xFFFu))
#define VERSION_VARIANT(v) (((v) >> 29u) & 0x7u)
#define VERSION_MAJOR(v)   (((v) >> 22u) & 0x7Fu)
#define VERSION_MINOR(v)   (((v) >> 12u) & 0x3FFu)
#define VERSION_PATCH(v)   ((v) & 0xFFFu)

#endif