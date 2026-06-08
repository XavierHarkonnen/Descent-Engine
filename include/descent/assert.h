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

#ifndef DESCENT_ASSERT_H
#define DESCENT_ASSERT_H

#if defined(DESCENT_DEBUG)
#define descent_assert(expression, message) ((expression) ? (void)0 : descent_assert_fail(#expression, message, __FILE__, __LINE__, __func__))
#else
#define descent_assert(expression, message) ((void)0)
#endif

_Noreturn void descent_assert_fail(
	const char *expression,
	const char *message,
	const char *file,
	unsigned int line,
	const char *function
);

#endif
