#ifndef DESCENT_LIB_FILE_UTILS_HPP
#define DESCENT_LIB_FILE_UTILS_HPP

#include <descent/type/core.hpp>
#include <descent/type/string.hpp>

namespace descent {

inline bool validate_name(string name) {
	if (!name || name.empty())
		return false;

	// Refuse if name is '.' or '..'
	if (name == string(".") || name == string(".."))
		return false;

	// Refuse if name contains non-printable ascii or a slash
	for (u64 i = 0; i < name.size(); ++i) {
		u8 c = u8(name[i]);

		if (c == '\0')
			break;

		if (c <= 31 || c >= 127 || c == '/')
			return false;
	}

	return true;
}

}

#endif
