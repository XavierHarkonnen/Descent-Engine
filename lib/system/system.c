#include <descent/sys.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline void sys_err_write(const char *s) {
	(void) write(STDERR_FILENO, s, strnlen(s, 1023));
}

#undef sys_fatal
#undef sys_assert

__attribute__((cold, noreturn))
void sys_fatal(const char *message, const char *file, const char *line) {
	sys_err_write(file);
	sys_err_write(":");
	sys_err_write(line);
	sys_err_write(": fatal error: ");
	sys_err_write(message ? message : "<no message>");
	sys_err_write("\n");

	_Exit(EXIT_FAILURE);
}

#if defined(DESCENT_DEBUG)
__attribute__((cold, noreturn))
void sys_assert(const char *expression, const char *message, const char *file, const char *line) {
	sys_err_write(file);
	sys_err_write(":");
	sys_err_write(line);
	sys_err_write(": assertion '");
	sys_err_write(expression);
	sys_err_write("' failed: ");
	sys_err_write(message ? message : "<no message>");
	sys_err_write("\n");

	_Exit(EXIT_FAILURE);
}
#endif
