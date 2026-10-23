#ifndef DESCENT_RENGINE_HPP
#define DESCENT_RENGINE_HPP

#include <descent/type/core.hpp>

struct wl_display;

namespace descent::rengine {

class Rengine {
	wl_display *_display;
	bool _init;

public:
	Rengine();

	~Rengine();

	explicit operator bool();
};

}

#endif