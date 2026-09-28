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

#ifndef DESCENT_SYSTEM_TIME_HPP
#define DESCENT_SYSTEM_TIME_HPP

#include <descent/type.hpp>

namespace descent::time {

constexpr u64 NANOSECONDS_PER_SECOND = 1000000000;
constexpr u64 INDEFINITE_TIMEOUT = 0;

u64 now(void);

u64 sleep(u64 duration);

}

#endif