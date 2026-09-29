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

#ifndef DESCENT_MEMORY_HPP
#define DESCENT_MEMORY_HPP

#include <descent/type.hpp>

#include <new>
// TODO: Don't use placement new

namespace descent::memory {

constexpr u64 CACHE_LINE_SIZE = 64;
constexpr u64 PAGE_SIZE = 4096;

constexpr u32 DYNAMIC_PAGES = 0;

enum class Access {
	NONE,
	READ,
	EDIT
};

class Detail {
protected:
	void *acquire(u32 page_count, Access access);
	void *resize(void *data, u32 page_count, u32 new_count);
	bool protect(void *data, u32 page_start, u32 page_count, Access access);
	void release(void *data, u32 pages);
};

template<typename T, u32 PAGES = DYNAMIC_PAGES>
class Region;

template<typename T, u32 PAGES>
class Region : public Detail {
	static_assert(__is_constructible(T), "T must be default-constructable");
	static_assert(PAGES != DYNAMIC_PAGES, "Cannot create static region with DYNAMIC_PAGES");

	Region(const Region &) = delete;
	Region &operator=(const Region &) = delete;

private:
	static constexpr u64 SIZE = PAGES * PAGE_SIZE;
	static constexpr u64 CAPACITY = SIZE / sizeof(T);
	
	static_assert(CAPACITY > 0, "Region has insufficient size to contain instance of type");
	static_assert(alignof(T) <= PAGE_SIZE, "Type has greater alignment than page alignment");

	T *_data;

public:
	Region() : _data(nullptr) {}

	Region(Region &&other) : _data(other._data) {
		other._data = nullptr;
	};

	~Region() {
		Region::release();
	}

	Region &operator=(Region &&other) {
		if (this != &other) {
			Region::release();

			_data = other._data;
			other._data = nullptr;
		}

		return *this;
	}

	explicit operator bool() const { 
		return _data; 
	}

	T &operator [](u64 index) {
		return _data[index];
	}
	
	const T &operator [](u64 index) const {
		return _data[index];
	}
	
	bool acquire() {
		if (_data)
			return false;
		
		_data = static_cast<T *>(Detail::acquire(PAGES, Access::EDIT));

		if (!_data)
			return false;

		for (u64 i = 0 ; i < capacity(); ++i)
			new (_data + i) T();

		return true;
	}

	void release() {
		if (_data) {
			for (u64 i = 1 ; i <= capacity(); ++i)
				(_data + capacity() - i)->~T();

			Detail::release(_data, pages());
			_data = nullptr;
		}
	}

	static constexpr u64 size() {
		return SIZE;
	}

	static constexpr u64 capacity() {
		return CAPACITY;
	}

	static constexpr u32 pages() {
		return PAGES;
	}
};

template<typename T>
class Region<T, DYNAMIC_PAGES> : public Detail {
private:

	u8 *_data;
	u64 _size;

	Region(const Region &) = delete;
	Region &operator=(const Region &) = delete;

public:
	Region() : _data(nullptr), _size(0) {}

	Region(u32 page_count) : Region() {
		Region::acquire(page_count);
	}

	Region(Region &&other) : _data(other._data), _size(other._size) {
		other._data = nullptr;
		other._size = 0;
	};

	~Region() {
		release();
	}

	Region &operator=(Region &&other) {
		if (this != &other) {
			release();
	
			_data = other._data;
			_size = other._size;
	
			other._data = nullptr;
			other._size = 0;
		}

		return *this;
	}

	explicit operator bool() const { 
		return _data; 
	}

	T &operator [](u64 index) {
		return _data[index];
	}
	
	const T &operator [](u64 index) const {
		return _data[index];
	}
	
	bool acquire(u32 page_count) {
		if (_data || page_count == 0)
			return false;
		
		_data = static_cast<T *>(Detail::acquire(page_count, Access::EDIT));
		
		if (!_data)
			return false;
		
		_size = page_count * PAGE_SIZE;
		for (u64 i = 0 ; i < capacity(); ++i)
			new (_data + i) T();

		return true;
	}
	
	bool resize(u32 page_count) {
		void *new_data = Detail::resize(_data, pages(), page_count);
		if (new_data) {
			_data = static_cast<T *>(new_data);
			_size = page_count * PAGE_SIZE;
			return true;
		}

		return false;
	}

	void release() {
		if (_data) {
			for (u64 i = 1 ; i <= capacity(); ++i)
				(_data + capacity() - i)->~T();

			Detail::release(_data, pages());
			_data = nullptr;
			_size = 0;
		}
	}

	u64 size() const {
		return _size;
	}

	u64 capacity() const {
		return size() / sizeof(T);
	}

	u32 pages() const {
		return _size / PAGE_SIZE;
	}
};

}

#endif
