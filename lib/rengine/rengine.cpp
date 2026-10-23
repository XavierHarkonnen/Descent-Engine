#include <descent/rengine/rengine.hpp>

extern "C" {
#include <vulkan/vulkan.h>
#include <wayland-client.h>
}

namespace descent::rengine {

Rengine::Rengine() : _init(false) {
	_display = wl_display_connect(nullptr);
	if (!_display)
		return;

	_init = true;
}

Rengine::~Rengine() {
	wl_display_disconnect(_display);
}

Rengine::operator bool() {
	return _init;
}

}
