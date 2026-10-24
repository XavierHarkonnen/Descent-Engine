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

#ifndef DESCENT_TIME_HPP
#define DESCENT_TIME_HPP

#include <descent/type/core.hpp>
#include <descent/type/traits.hpp>

namespace descent::time {

class duration {
private:
	i64 _ns;

	constexpr explicit duration(i64 ns) : _ns(ns) {}

public:
	constexpr duration() : _ns(0) {}

	static constexpr duration ns(i64 ns) { return duration(ns); }
	static constexpr duration us(i64 us) { return ns(us * 1000); }
	static constexpr duration ms(i64 ms) { return us(ms * 1000); }
	static constexpr duration s(i64 s) { return ms(s * 1000); }

	constexpr duration operator +(const duration d) const {
		return duration(ns() + d.ns());
	}

	constexpr duration operator -(const duration o) const {
		return duration(ns() - o.ns());
	}

	template<typename T>
	constexpr duration operator *(T scale) const {
		static_assert(type::traits<T>::is_numeric, "Attempted to scale duration with non-numeric type");
		return duration(i64(ns() * scale));
	}

	template<typename T>
	constexpr duration operator /(T scale) const {
		static_assert(type::traits<T>::is_numeric, "Attempted to scale duration with non-numeric type");
		return duration(i64(ns() / scale));
	}

	constexpr bool operator ==(duration o) const { return _ns == o._ns; }
	constexpr bool operator !=(duration o) const { return _ns != o._ns; }
	constexpr bool operator <(duration o) const { return _ns < o._ns; }
	constexpr bool operator <=(duration o) const { return _ns <= o._ns; }
	constexpr bool operator >(duration o) const { return _ns > o._ns; }
	constexpr bool operator >=(duration o) const { return _ns >= o._ns; }

	constexpr i64 ns() const { return _ns; }
	constexpr i64 us() const { return ns() / 1000; }
	constexpr i64 ms() const { return us() / 1000; }
	constexpr i64 s() const { return ms() / 1000; }
};

class monotonic {
private:
	i64 _ns;

	constexpr explicit monotonic(i64 ns) : _ns(ns) {}

public:
	static monotonic now();

	constexpr monotonic operator +(const duration d) const {
		return monotonic(ns() + d.ns());
	}

	constexpr monotonic operator -(const duration d) const {
		return monotonic(ns() - d.ns());
	}

	constexpr duration operator -(const monotonic o) const {
		return duration::ns(ns() - o.ns());
	}

	constexpr bool operator == (monotonic o) const { return _ns == o._ns; }
	constexpr bool operator != (monotonic o) const { return _ns != o._ns; }
	constexpr bool operator <  (monotonic o) const { return _ns <  o._ns; }
	constexpr bool operator <= (monotonic o) const { return _ns <= o._ns; }
	constexpr bool operator >  (monotonic o) const { return _ns >  o._ns; }
	constexpr bool operator >= (monotonic o) const { return _ns >= o._ns; }

	constexpr i64 ns() const { return _ns; }
	constexpr i64 us() const { return ns() / 1000; }
	constexpr i64 ms() const { return us() / 1000; }
	constexpr i64 s() const { return ms() / 1000; }
};

class timestamp {
private:
	i64 _ns;

	constexpr explicit timestamp(i64 ns) : _ns(ns) {}

public:
	static timestamp now();

	constexpr bool operator == (timestamp o) const { return _ns == o._ns; }
	constexpr bool operator != (timestamp o) const { return _ns != o._ns; }
	constexpr bool operator <  (timestamp o) const { return _ns <  o._ns; }
	constexpr bool operator <= (timestamp o) const { return _ns <= o._ns; }
	constexpr bool operator >  (timestamp o) const { return _ns >  o._ns; }
	constexpr bool operator >= (timestamp o) const { return _ns >= o._ns; }

	constexpr i64 ns() const { return _ns; }
	constexpr i64 us() const { return ns() / 1000; }
	constexpr i64 ms() const { return us() / 1000; }
	constexpr i64 s() const { return ms() / 1000; }
};

void sleep(duration d);

void sleep(monotonic deadline);

}

#endif
