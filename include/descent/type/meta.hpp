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

#ifndef DESCENT_TYPE_META_HPP
#define DESCENT_TYPE_META_HPP

namespace descent::type {

template<typename T>
struct remove_reference {
	using type = T;
};

template<typename T>
struct remove_reference<T &> {
	using type = T;
};

template<typename T>
struct remove_reference<T &&> {
	using type = T;
};

template<typename T>
struct remove_const {
	using type = T;
};

template<typename T>
struct remove_const<const T> {
	using type = T;
};

template<typename T>
struct remove_volatile {
	using type = T;
};

template<typename T>
struct remove_volatile<volatile T> {
	using type = T;
};

template<typename T>
struct remove_cv {
	using type = typename remove_volatile<typename remove_const<T>::type>::type;
};

template<typename T>
struct decay {
private:
	using unr = typename remove_reference<T>::type;

public:
	using type = typename remove_cv<unr>::type;
};

template<typename T>
struct is_lvalue_reference {
	static constexpr bool value = false;
};

template<typename T>
struct is_lvalue_reference<T &> {
	static constexpr bool value = true;
};

template<bool B, typename T, typename F>
struct conditional {
	using type = T;
};

template<typename T, typename F>
struct conditional<false, T, F> {
	using type = F;
};

template<typename...>
using void_t = void;

}

#endif
