#include "file.hpp"

extern "C" {
#include <unistd.h>
}

namespace descent::fs {

Handle::~Handle() {
	if (valid()) {
		close(h);
	}
}

}
