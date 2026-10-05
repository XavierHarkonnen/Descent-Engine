#ifndef DESCENT_MEMORY_CONSTRUCT_HPP
#define DESCENT_MEMORY_CONSTRUCT_HPP

#include <descent/type/core.hpp>

namespace descent::memory {

class pnewp {
	template<class T, class... Args>
	friend T *construct(T* storage, Args&&... args);
	
private:
	void *_p;
	explicit pnewp(void *p) : _p(p) {}

public:
	void *get() const { return _p; }
};

}

inline void *operator new(__SIZE_TYPE__, const descent::memory::pnewp &p) { return p.get(); }

namespace descent::memory {

template<class T, class... Args>
T *construct(T* storage, Args&&... args) {
	return new (pnewp(storage)) T(static_cast<Args &&>(args)...);
}

}

#endif