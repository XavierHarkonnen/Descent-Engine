#ifndef DESCENT_BSL_SCHEMA_HPP
#define DESCENT_BSL_SCHEMA_HPP

#include <descent/bsl/serial.hpp>
#include <descent/type/core.hpp>
#include <descent/type/traits.hpp>

namespace descent::bsl {

class SchemaData {
	friend bool deserialize(SchemaData &schema, const u8* input, u64 size, u64& offset);

public:
	static constexpr u64 MAX_PARAMETERS = 22;

protected:
	u8 _count = 0;
	type::Type _types[MAX_PARAMETERS] = {};

public:
	constexpr u8 count() const {
		return _count;
	}

	constexpr const type::Type *types() const {
		return _types;
	}
};

template<typename ...Args>
class Schema : public SchemaData {
public:
	static_assert(sizeof...(Args) <= MAX_PARAMETERS, "BSL accepts at most MAX_PARAMETERS parameters");

public:
	Schema() {
		u64 i = 0;
		_count = sizeof...(Args);
		((_types[i++] = type::traits<typename type::decay<Args>::type>::type), ...);
	}
};

inline bool serialize(const SchemaData &schema, u8* output, u64 size, u64& offset) {
	u64 cursor = offset;

	if (!serialize(schema.count(), output, size, cursor))
		return false;

	for (u64 i = 0; i < schema.count(); ++i)
		if (!serialize(schema.types()[i], output, size, cursor))
			return false;

	offset = cursor;
	return true;
}

inline bool deserialize(SchemaData &schema, const u8* input, u64 size, u64& offset) {
	u64 cursor = offset;
	u8 count;

	if (!deserialize(count, input, size, cursor))
		return false;

	if (count > SchemaData::MAX_PARAMETERS)
		return false;

	schema._count = count;

	for (u64 i = 0; i < count; ++i)
		if (!deserialize(schema._types[i], input, size, cursor))
			return false;

	offset = cursor;
	return true;
}

}

#endif