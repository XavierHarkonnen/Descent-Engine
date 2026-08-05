#ifndef DESCENT_RCODE_H
#define DESCENT_RCODE_H

#include <descent/type/core.h>

// Type

typedef u16 rcode;

// Internal Constants

#define RCODE_WIDTH          16
#define RCODE_WIDTH_SEVERITY  2
#define RCODE_WIDTH_MODULE    6
#define RCODE_WIDTH_CODE      8

#define RCODE_MASK_SEVERITY ((1U << RCODE_WIDTH_SEVERITY) - 1)
#define RCODE_MASK_MODULE   ((1U << RCODE_WIDTH_MODULE  ) - 1)
#define RCODE_MASK_CODE     ((1U << RCODE_WIDTH_CODE    ) - 1)

#define RCODE_SHIFT_SEVERITY (RCODE_WIDTH          - RCODE_WIDTH_SEVERITY)
#define RCODE_SHIFT_MODULE   (RCODE_SHIFT_SEVERITY - RCODE_WIDTH_MODULE)
#define RCODE_SHIFT_CODE     (RCODE_SHIFT_MODULE   - RCODE_WIDTH_CODE)

_Static_assert(RCODE_SHIFT_CODE == 0, "RCODE_SHIFT_CODE must be zero");

// Constructor

#define RCODE(severity, module, code) ( \
	(rcode) (((rcode) (severity) & RCODE_MASK_SEVERITY) << RCODE_SHIFT_SEVERITY) | \
	(rcode) (((rcode) (module)   & RCODE_MASK_MODULE)   << RCODE_SHIFT_MODULE)   | \
	(rcode) (((rcode) (code)     & RCODE_MASK_CODE)     << RCODE_SHIFT_CODE)       \
)

// Severities

#define RCODE_SEVERITY_X_LIST \
	X(RCODE_SEVERITY_SUCCESS, 0) /*The operation succeeded, potentially with some additional information*/ \
	X(RCODE_SEVERITY_WARN,    1) /*The operation completed in a way which likely needs special handling*/ \
	X(RCODE_SEVERITY_ERROR,   2) /*The operation failed in a way which likely can be handled by the program*/ \
	X(RCODE_SEVERITY_FATAL,   3) /*The operation failed in a way which likely cannot be handled by the program*/

enum {
#define X(name, value) name = value,
	RCODE_SEVERITY_X_LIST
#undef X
};

// Modules

#define RCODE_MODULE_X_LIST \
	X(RCODE_MODULE_GENERAL, 0) \
	X(RCODE_MODULE_CORE,    1) \
	X(RCODE_MODULE_FILE,    2) \
	X(RCODE_MODULE_RANDOM,  3) \
	X(RCODE_MODULE_RENGINE, 4) \
	X(RCODE_MODULE_SYSTEM,  5)

enum {
#define X(name, value) name = value,
	RCODE_MODULE_X_LIST
#undef X
};

// Codes

#define RCODE_X_LIST \
	/* General Module */ \
	X(DESCENT_SUCCESS,          RCODE(RCODE_SEVERITY_SUCCESS, RCODE_MODULE_GENERAL, 0), "Unqualified success") \
	X(DESCENT_ERROR_NULL,       RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_GENERAL, 1), "Null pointer provided as argument") \
	X(DESCENT_ERROR_INVALID,    RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_GENERAL, 2), "Invalid value provided as argument") \
	/* File Module */ \
	X(FILE_SUCCESS_EOF,         RCODE(RCODE_SEVERITY_SUCCESS, RCODE_MODULE_FILE,    0), "End of file") \
	X(FILE_ERROR_ROOT,          RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    0), "Path has a bad root") \
	X(FILE_ERROR_PATH_EMPTY,    RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    1), "Path contains an invalid character") \
	X(FILE_ERROR_PATH_CHAR,     RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    2), "Path contains an invalid character") \
	X(FILE_ERROR_PATH_ABSOLUTE, RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    3), "Path is empty") \
	X(FILE_ERROR_PATH_INVALID,  RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    4), "Path is invalid") \
	X(FILE_ERROR_MODE,          RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    5), "File mode is invalid") \
	/*...*/\
	X(FILE_ERROR_PATH_LENGTH,   RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    6), "Path is too long") \
	X(FILE_ERROR_PATH_DEPTH,    RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    7), "Path contains too many segments") \
	X(FILE_ERROR_PATH_ESCAPE,   RCODE(RCODE_SEVERITY_ERROR,   RCODE_MODULE_FILE,    8), "Path would escape root") \

enum {
#define X(name, value, description) name = value,
	RCODE_X_LIST
#undef X
};

_Static_assert(DESCENT_SUCCESS == 0, "DESCENT_SUCCESS must equal zero");

// Getters

#define RCODE_SEVERITY(r) (((rcode) (r) >> RCODE_SHIFT_SEVERITY) & RCODE_MASK_SEVERITY)
#define RCODE_MODULE(r)   (((rcode) (r) >> RCODE_SHIFT_MODULE)   & RCODE_MASK_MODULE)
#define RCODE_CODE(r)     (((rcode) (r) >> RCODE_SHIFT_CODE)     & RCODE_MASK_CODE)

// Classifiers

#define RCODE_IS_SUCCESS(r) (RCODE_SEVERITY(r) == RCODE_SEVERITY_SUCCESS)
#define RCODE_IS_WARNING(r) (RCODE_SEVERITY(r) == RCODE_SEVERITY_WARN)
#define RCODE_IS_ERROR(r)   (RCODE_SEVERITY(r) == RCODE_SEVERITY_ERROR)
#define RCODE_IS_FATAL(r)   (RCODE_SEVERITY(r) == RCODE_SEVERITY_FATAL)

// Thresholds

#define RCODE_SUCCEEDED(r) (RCODE_SEVERITY(r) == RCODE_SEVERITY_SUCCESS)
#define RCODE_COMPLETED(r) (RCODE_SEVERITY(r) <= RCODE_SEVERITY_WARN)
#define RCODE_ATTENTION(r) (RCODE_SEVERITY(r) >= RCODE_SEVERITY_WARN)
#define RCODE_FAILED(r)    (RCODE_SEVERITY(r) >= RCODE_SEVERITY_ERROR)

// Debug Helpers

const char *rcode_severity_string(rcode r);
const char *rcode_module_string(rcode r);
const char *rcode_string(rcode r);
const char *rcode_description(rcode r);

// Define RCODE_ITERATOR before including to access X macros

#ifndef RCODE_ITERATOR
#undef RCODE_SEVERITY_X_LIST
#undef RCODE_MODULE_X_LIST
#undef RCODE_X_LIST
#endif

#endif