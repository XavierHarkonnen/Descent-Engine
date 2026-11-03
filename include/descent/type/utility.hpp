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

#ifndef DESCENT_TYPE_UTILITY_HPP
#define DESCENT_TYPE_UTILITY_HPP

#include <descent/type/core.hpp>
#include <descent/type/meta.hpp>

namespace descent {

template<typename T>
[[gnu::always_inline]]
constexpr typename type::remove_reference<T>::type &&move(T &&value) {
	return static_cast<typename type::remove_reference<T>::type &&>(value);
}

template<typename T>
[[gnu::always_inline]]
constexpr T &&forward(typename type::remove_reference<T>::type &value) {
	return static_cast<T &&>(value);
}

template<typename T>
[[gnu::always_inline]]
constexpr T &&forward(typename type::remove_reference<T>::type &&value) {
	static_assert(!type::is_lvalue_reference<T>::value, "Attempted to forward an rvalue as an lvalue");
	return static_cast<T &&>(value);
}

// Call unqualified so a type's own swap is found by ADL; descent::swap(a, b) always uses this one.
// Inside a class with a member swap, add `using descent::swap;` first
template<typename T>
constexpr void swap(T &a, T &b) {
	T temp = move(a);
	a      = move(b);
	b      = move(temp);
}

template<typename T, u64 N>
constexpr void swap(T (&a)[N], T (&b)[N]) {
	for (u64 i = 0; i < N; ++i)
		swap(a[i], b[i]);
}

}

#endif
