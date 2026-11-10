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

#include <descent/fs.hpp>

#include <descent/memory/stack.hpp>

#include "../name.hpp"
#include "system.hpp"

namespace descent::fs {

File::File(const Directory &parent, string<> name, Access a) {
	const u64 LENGTH = name::length(name);

	if (!parent || LENGTH == 0)
		return;

	if (LENGTH < name.size()) {
		h = Handle(file::open(parent.handle().h, name.data(), a));
		return;
	}

	auto buffer = memory::stack::Allocator::copy(name);
	if (!buffer)
		return;

	h = Handle(file::open(parent.handle().h, buffer.data(), a));
}

u64 File::read(u64 offset, view<u8> out) const {
	u64 read_count = 0;

	do {
		ssize_t result = ::pread(handle().h, out.data(), out.size(), offset);
		if (result < 0)
			return 0;

		read_count += u64(result);
	} while (true);

	return read_count;
}

}
