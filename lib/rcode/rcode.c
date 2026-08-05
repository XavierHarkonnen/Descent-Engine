#define RCODE_ITERATOR
#include <descent/rcode.h>

#include <descent/utils.h>

const char *rcode_severity_string(rcode r) {
	switch (RCODE_SEVERITY(r)) {
#define X(name, value) case name: return STRINGIFY(name);
		RCODE_SEVERITY_X_LIST
#undef X
	default: __builtin_unreachable();
	}
}

const char *rcode_module_string(rcode r) {
	switch (RCODE_MODULE(r)) {
#define X(name, value) case name: return STRINGIFY(name);
		RCODE_MODULE_X_LIST
#undef X
	default: return "Unknown module";
	}
}

const char *rcode_string(rcode r) {
	switch (r) {
#define X(name, value, description) case name: return STRINGIFY(name);
		RCODE_X_LIST
#undef X
	default: return "Unknown result code";
	}
}

const char *rcode_description(rcode r) {
	switch (r) {
#define X(name, value, description) case name: return description;
		RCODE_X_LIST
#undef X
	default: return "Unknown result code";
	}
}