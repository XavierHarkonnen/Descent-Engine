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

#ifndef DESCENT_BSL_SITE_HPP
#define DESCENT_BSL_SITE_HPP

#include <descent/bsl/runtime.hpp>
#include <descent/bsl/schema.hpp>
#include <descent/bsl/serial.hpp>
#include <descent/system.hpp>
#include <descent/type/bit.hpp>
#include <descent/type/core.hpp>
#include <descent/type/traits.hpp>

namespace descent::bsl {

class SiteData {
	friend bool deserialize(SiteData &schema, const u8 *input, u64 size, u64 &offset);

protected:
	string _message;
	string _file;
	u32 _line;
	u32 _index;
	SchemaData _schema;
	Level _level;

	SiteData() : _message(nullptr), _file(nullptr), _line(0), _index(0), _level(Level::NONE) {}

public:
	string message() const {
		return string(_message);
	}

	string file() const {
		return string(_file);
	}

	u32 line() const {
		return _line;
	}

	Level level() const {
		return _level;
	}

	const SchemaData &schema() const {
		return _schema;
	}
};

template<typename... Args>
class Site : public SiteData {

private:
	static void entry();

	static void parameter(void *p, u64 size);

	template<typename T>
	static void parameter(T p) {
		static_assert(type::traits<T>::type != type::Type::INVALID, "Attempted to log unsupported type");
		parameter(&p, sizeof(T));
	}

public:
	Site(Level level, string message, string file = string(__builtin_FILE()), u32 line = __builtin_LINE()) {
		extern const SiteData __start_bsl_sites[];
		extern const SiteData __stop_bsl_sites[];

		const u64 start = reinterpret_cast<u64>(__start_bsl_sites);
		const u64 stop  = reinterpret_cast<u64>(__stop_bsl_sites);
		const u64 self  = reinterpret_cast<u64>(this);

		if (self < start || self > stop || sizeof(*this) > stop - self)
			return;

		const u64 index = (self - start) / sizeof(Site);

		sys_assert(index <= U32_MAX, "Site index exceeds the supported u32 range");
		sys_assert(bit::popcnt(u8(level)) == 1, "Logging can only be performed at a single level");
		sys_assert(message.data() != nullptr, "Logging cannot be performed with a null message");
		sys_assert(file.data() != nullptr, "Logging cannot be performed with a null file");

		_message = message;
		_file    = file;
		_line    = line;
		_index   = u32(index);
		_schema  = Schema<Args...>();
		_level   = level;
	}

	void log(Args... args) const {
		if (enabled(_level)) {
			entry();
			(parameter(args), ...);
		}
	}
};

inline bool serialize(const SiteData &site, u8 *output, u64 size, u64 &offset) {
	u64 cursor = offset;

	if (!serialize(site.message(), output, size, cursor))
		return false;

	if (!serialize(site.file(), output, size, cursor))
		return false;

	if (!serialize(site.line(), output, size, cursor))
		return false;

	if (!serialize(site.schema(), output, size, cursor))
		return false;

	if (!serialize(site.level(), output, size, cursor))
		return false;

	offset = cursor;
	return true;
}

inline bool deserialize(SiteData &site, const u8 *input, u64 size, u64 &offset) {
	u64 cursor = offset;

	if (!deserialize(site._message, input, size, cursor))
		return false;

	if (!deserialize(site._file, input, size, cursor))
		return false;

	if (!deserialize(site._line, input, size, cursor))
		return false;

	if (!deserialize(site._schema, input, size, cursor))
		return false;

	if (!deserialize(site._level, input, size, cursor))
		return false;

	offset = cursor;
	return true;
}

}

#endif
