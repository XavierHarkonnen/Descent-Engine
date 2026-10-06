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
#include <descent/memory/construct.hpp>

namespace descent::memory::stack {

template<typename T>
class Box {
	friend class Allocator;

	Box() = delete;
	Box(const Box&) = delete;
	Box& operator=(const Box&) = delete;

	void clear();

private:
	T *_data;
	void *_meta;
	u64 _size;

	template<typename ...Args>
	Box(T* data, void *meta, u64 size, Args &&...args) : _data(data), _meta(meta), _size(size) {
		for (u64 index = 0; index < _size; ++index) {
			construct(_data + index, static_cast<Args &&>(args)...);
		}
	}

	Box(Box &&other) : _data(other._data), _meta(other._meta), _size(other._size) {
		other._data = nullptr;
		other._meta = nullptr;
		other._size = 0;
	}

	Box& operator=(Box &&other) {
		if (this != &other) {
			clear();

			_data = other._data;
			_meta = other._meta;
			_size = other._size;

			other._data = nullptr;
			other._meta = nullptr;
			other._size = 0;
		}
		return *this;
	}

public:
	~Box() {
		clear();
	}

	explicit operator bool() const {
		return _data;
	}

	T* data() {
		return _data;
	}

	const T* data() const {
		return _data;
	}

	u64 size() const {
		return _size;
	}

	T& operator[](u64 index) {
		return data()[index];
	}

	const T& operator[](u64 index) const {
		return data()[index];
	}
};

class Allocator {
	template<typename T>
	friend class Box;
private:
	static void create(void *&data, void *&meta, u32 size, u32 align, u32 count);

	static void clear(void *data, void *meta);

public:
	template<typename T, typename ...Args>
	static Box<T> create(u64 count, Args &&...args) {
		static_assert(sizeof(T) <= U32_MAX, "Size of object is too large");
		static_assert(alignof(T) <= U32_MAX, "Alignment of object is too large");
		
		void* data = nullptr;
		void* meta = nullptr;
		
		if (count != 0 && count <= U32_MAX)
			create(data, meta, sizeof(T), alignof(T), count);

		return Box<T>(static_cast<T*>(data), meta, count, static_cast<Args &&>(args)...);
	}
};

template<typename T>
void Box<T>::clear() {
	if (*this) {
		for (u64 i = 0; i < size(); ++i)
			_data[i].~T();
	
		Allocator::clear(_data, _meta);
	
		_data = nullptr;
		_meta = nullptr;
		_size = 0;
	}
}

}

#endif