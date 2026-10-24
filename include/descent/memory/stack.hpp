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

#ifndef DESCENT_MEMORY_STACK_HPP
#define DESCENT_MEMORY_STACK_HPP

#include <descent/type/core.hpp>
#include <descent/type/math.hpp>
#include <descent/type/string.hpp>
#include <descent/memory/construct.hpp>

namespace descent::memory::stack {

// A Box owns objects in its thread's stack allocator, a LIFO bump allocator
// private to each thread. A Box is bound to the scope that receives it from
// Allocator:
//
// - It must be a local variable on the stack of the thread that created it,
//   and must be destroyed by that thread. It must never be static,
//   thread_local, heap-allocated or a member of an object stored in any of
//   those places.
// - It must not outlive its thread, nor be shared with another thread.
// - Pointers and references into it, from data() or operator[], must not
//   outlive it. Other threads may use them only while it is alive.
//
// A function may return the result of Allocator directly, as Allocator does;
// the Box then belongs to the caller's scope.
//
// Enforced in every build: a Box cannot be copied, moved by user code, or
// created or destroyed with a class new-expression or delete-expression.
//
// Checked in debug builds only (DESCENT_DEBUG), at no cost to release builds:
// - A Box that owns objects lies on the calling thread's stack whenever it is
//   created, moved into or destroyed.
// - A Box is freed into the allocator that allocated it.
// - A thread other than the main thread holds no allocations when it exits.
//
// Not checked: a Box made with a global placement new-expression (::new) in
// stack storage and never destroyed, while its thread keeps running; a Box
// left alive when the main thread calls exit(); and pointers into a Box that
// escape its scope.
template<typename T>
class Box {
	friend class Allocator;

	Box(const Box &)             = delete;
	Box &operator =(const Box &) = delete;

	void clear();

	void local() const;

private:
	T *_data;
	void *_meta;
	u64 _size;
	u64 _capacity;

	Box() : _data(nullptr), _meta(nullptr), _size(0), _capacity(0) {}

	Box(T *data, void *meta, u64 capacity) : _data(data), _meta(meta), _size(0), _capacity(capacity) {
		local();
	}

	Box(Box &&other) : _data(other._data), _meta(other._meta), _size(other._size), _capacity(other._capacity) {
		if (_data || _meta)
			local();

		other._data     = nullptr;
		other._meta     = nullptr;
		other._size     = 0;
		other._capacity = 0;
	}

	Box &operator =(Box &&other) {
		if (this != &other) {
			clear();

			_data     = other._data;
			_meta     = other._meta;
			_size     = other._size;
			_capacity = other._capacity;

			if (_data || _meta)
				local();

			other._data     = nullptr;
			other._meta     = nullptr;
			other._size     = 0;
			other._capacity = 0;
		}
		return *this;
	}

	template<typename... Args>
	void init(Args &&...args) {
		if (*this) {
			for (u64 index = 0; index < _capacity; ++index) {
				construct(_data + index, static_cast<Args &&>(args)...);
				++_size;
			}
		}
	}

	void copy(const T *source, u64 count) {
		if (*this) {
			u64 limit = math::min(count, _capacity);

			for (u64 index = 0; index < limit; ++index) {
				construct(_data + index, source[index]);
				++_size;
			}
		}
	}

public:
	// A Box exists only on its thread's stack
	static void *operator new(__SIZE_TYPE__)           = delete;
	static void *operator new(__SIZE_TYPE__, void *)   = delete;
	static void *operator new[](__SIZE_TYPE__)         = delete;
	static void *operator new[](__SIZE_TYPE__, void *) = delete;
	static void operator delete(void *)                = delete;
	static void operator delete[](void *)              = delete;

	~Box() {
		clear();
	}

	explicit operator bool() const {
		return _data;
	}

	T *data() {
		return _data;
	}

	const T *data() const {
		return _data;
	}

	u64 size() const {
		return _size;
	}

	u64 capacity() const {
		return _capacity;
	}

	T &operator [](u64 index) {
		return data()[index];
	}

	const T &operator [](u64 index) const {
		return data()[index];
	}
};

class Allocator {
	template<typename T>
	friend class Box;
private:
	static bool create(void *&data, void *&meta, u32 size, u32 align, u32 capacity);

	static void clear(void *data, void *meta);

#if defined(DESCENT_DEBUG)
	static void local(const void *box);
#endif

public:
	template<typename T, typename... Args>
	static Box<T> create(u64 capacity, Args &&...args) {
		static_assert(sizeof(T) <= U32_MAX, "Size of object is too large");
		static_assert(alignof(T) <= U32_MAX, "Alignment of object is too large");

		if (capacity == 0 || capacity > U32_MAX)
			return {};

		void *data = nullptr;
		void *meta = nullptr;

		if (!create(data, meta, sizeof(T), alignof(T), capacity))
			return {};

		Box<T> result(static_cast<T *>(data), meta, capacity);
		result.init(static_cast<Args &&>(args)...);
		return result;
	}

	template<typename T>
	static Box<T> copy(u64 capacity, const T *source, u64 count) {
		static_assert(sizeof(T) <= U32_MAX, "Size of object is too large");
		static_assert(alignof(T) <= U32_MAX, "Alignment of object is too large");

		if (capacity == 0 || capacity > U32_MAX || source == nullptr || count == 0 || count > capacity)
			return {};

		void *data = nullptr;
		void *meta = nullptr;

		if (!create(data, meta, sizeof(T), alignof(T), u32(capacity)))
			return {};

		Box<T> result(static_cast<T *>(data), meta, capacity);
		result.copy(source, count);
		return result;
	}

	static Box<char> copy(string s) {
		if (!s)
			return {};

		const u64 SIZE   = s.size();
		Box<char> result = copy<char>(SIZE + 1, s.data(), SIZE);
		if (result)
			result[SIZE] = '\0';

		return result;
	}
};

template<typename T>
void Box<T>::local() const {
#if defined(DESCENT_DEBUG)
	Allocator::local(this);
#endif
}

template<typename T>
void Box<T>::clear() {
	if (_data || _meta)
		local();

	if (_data) {
		for (u64 i = 0; i < _size; ++i)
			_data[i].~T();
	}

	if (_data || _meta)
		Allocator::clear(_data, _meta);

	_data     = nullptr;
	_meta     = nullptr;
	_size     = 0;
	_capacity = 0;
}

}

#endif
