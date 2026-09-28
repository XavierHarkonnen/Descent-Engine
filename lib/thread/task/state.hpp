#ifndef DESCENT_LIB_THREAD_STATE_HPP
#define DESCENT_LIB_THREAD_STATE_HPP

#include <descent/type.hpp>

namespace descent::thread::task {
	class Root;
	class Worker;
	class Pool;
}

namespace descent::thread::task::state {
	Worker *self();
	void self(Worker *);
};

#endif