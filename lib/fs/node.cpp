#ifndef DESCENT_LIB_FS_NODE_HPP
#define DESCENT_LIB_FS_NODE_HPP

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
}

#include <descent/access.hpp>
#include <descent/thread/atomic.hpp>
#include <descent/thread/rwlock.hpp>
#include <descent/type/core.hpp>

#include "data.hpp"
#include "directory.hpp"
#include "file.hpp"

namespace descent::fs {

// Interface

class File {
private:
	u8 node;
public:
};

class Directory {
private:
	u8 node;
public:
};

// Implementation

enum class Type {
	NONE,
	FILE,
	DIRECTORY
};

class Node {
	friend class Nodes;

private:
	enum : u8 {
		FLAG_TYPE   = u8(1) << 0,
		FLAG_ACCESS = u8(1) << 1,
	};

	u64 _dev = 0;
	u64 _ino = 0;

	thread::Atomic<u64> _size{0};

	thread::RWLock _structural;
	thread::RWLock _content;

	i32 _fd = -1;

	thread::Atomic<u16> _refs{0};

	u16 _name;

	u8 _flags = 0;

	u8 _parent = 0;

	u8 _pad[22]; // Padding can be used for additional metadata; could use SSO for names

	bool type(Type t);

	bool access(Access t);

public:
	bool live() const;

	Type type() const;

	Access access() const;

	bool open(u8 parent, u16 name, Type t, Access a);

	bool bind(i32 fd);

	void close();
};

class Nodes {
private:
	static constexpr u64 NAMES_SIZE = u64(1) << 16;
	static constexpr u64 MAX_NODES = 256;
	
	char _names[NAMES_SIZE];
	Node _nodes[MAX_NODES];

public:
	char *name(u16 index) {
		if (index == U16_MAX)
			return nullptr;

		return &_names[index];
	}

	Node &node(u8 index) {
		return _nodes[index];
	}
};

static Nodes nodes;

bool Node::live() const {
	return _fd >= 0;
}

Type Node::type() const {
	if (!live())
		return Type::NONE;

	return (_flags & FLAG_TYPE) ? Type::FILE : Type::DIRECTORY;
}

bool Node::type(Type t) {
	if (t == Type::FILE)
		_flags |= FLAG_TYPE;
	else if (t == Type::DIRECTORY)
		_flags &= ~FLAG_TYPE;
	else
		return false;

	return true;
}

Access Node::access() const {
	if (!live())
		return Access::NONE;
	
	return (_flags & FLAG_ACCESS) ? Access::EDIT : Access::READ;
}

bool Node::access(Access a) {
	if (a == Access::EDIT)
		_flags |= FLAG_ACCESS;
	else if (a == Access::READ)
		_flags &= ~FLAG_ACCESS;
	else
		return false;

	return true;
}

bool Node::open(u8 parent, u16 name, Type t, Access a) {
	Data data;

	if (!type(t))
		goto error;

	if (!access(a))
		goto error;

	switch (t) {
		case Type::FILE:
			data = file::open(nodes.node(parent)._fd, nodes.name(name), a);
		case Type::DIRECTORY:
			data = directory::open(nodes.node(parent)._fd, nodes.name(name));
		default:
			goto error;
	}

	_name = name;

	_dev = u64(st.st_dev);
	_ino = u64(st.st_ino);

	_size.store(u64(st.st_size), thread::atomic::Order::RELEASE);

	_fd = fd;

	_parent = parent;

	return true;

	error: {
		::close(fd);
		return false;
	}
}

bool Node::bind(i32 fd) {
	i32 old = _fd;

	struct stat st;
	if (::fstat(fd, &st) < 0)
		goto error;

	_content.wlock();

	old = _fd;
	_fd = fd;

	_dev = u64(st.st_dev);
	_ino = u64(st.st_ino);

	_size.store(u64(st.st_size), thread::atomic::Order::RELEASE);

	_content.wunlock();

	return old;

	error: {
		::close(fd);
		return false;
	}
}

void Node::close() {
	_structural.wlock();
	_content.wlock();

	if (live()) {
		_name = U16_MAX;

		_dev = 0;
		_ino = 0;

		_size.store(0, thread::atomic::Order::RELAXED);

		::close(_fd);
		_fd = -1;

		_flags = 0;

		_parent = 0;
	}

	_content.wunlock();
	_structural.wunlock();
}

}

#endif
