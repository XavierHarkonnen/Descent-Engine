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

#ifndef DESCENT_SYS_HPP
#define DESCENT_SYS_HPP

#include <descent/utils.hpp>

__attribute__((cold, noreturn))
void sys_fatal(const char *message, const char *file, const char *line);
#define sys_fatal(message) sys_fatal(message, __FILE__, STRINGIFY(__LINE__))

#if defined(DESCENT_DEBUG)
__attribute__((cold, noreturn))
#define sys_trap(message) sys_fatal(message)
#else
#define sys_trap(message) ((void)0)
#endif

#if defined(DESCENT_DEBUG)
__attribute__((cold, noreturn))
void sys_assert(const char *expression, const char *message, const char *file, const char *line);
#define sys_assert(expression, message) ((expression) ? (void)0 : sys_assert(STRINGIFY(expression), message, __FILE__, STRINGIFY(__LINE__)))
#else
#define sys_assert(expression, message) ((void)0)
#endif

#endif
