#ifndef DESCENT_LIB_FS_NODE_HPP
#define DESCENT_LIB_FS_NODE_HPP

#include <descent/type/core.hpp>
#include <descent/thread/atomic.hpp>
#include <descent/thread/rwlock.hpp>

#include "name.hpp"

namespace descent::fs {

class Node {
public:
private:
	enum : u8 {
		FLAG_TYPE  = u8(1) << 0,
		FLAG_WRITE = u8(1) << 1,
	};

	i32 fd     = -1;
	i32 parent = -1;
	u64 dev    = 0;
	u64 ino    = 0;
	thread::Atomic<u64> size;
	u32 refs   = 0;
	u8 flags;
	char name[name::MAX_NAME_LENGTH + 1] = {};

public:
	bool live();

	void open(i32 descriptor);

	void close();
};

class Nodes {
	static constexpr u64 MAX = 256;
	Node nodes[MAX];
};

bool Node::live() {

}

void Node::close() {

}

}

#endif
