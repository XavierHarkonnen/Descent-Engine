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

#ifndef DESCENT_MEMORY_CONSTRUCT_HPP
#define DESCENT_MEMORY_CONSTRUCT_HPP

#include <descent/type/core.hpp>

namespace descent::memory {

class pnewp {
	template<class T, class... Args>
	friend T *construct(T* storage, Args&&... args);
	
private:
	void *_p;
	explicit pnewp(void *p) : _p(p) {}

public:
	void *get() const { return _p; }
};

}

inline void *operator new(__SIZE_TYPE__, const descent::memory::pnewp &p) { return p.get(); }

namespace descent::memory {

// Construct a 
template<class T, class... Args>
T *construct(T* storage, Args&&... args) {
	return new (pnewp(storage)) T(static_cast<Args &&>(args)...);
}

}

#endif