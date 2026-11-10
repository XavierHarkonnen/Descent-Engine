#include "fs.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace descent::fs {

// Implementation
//
// Locking. Every path that holds more than one lock takes them in this order:
//
//     Nodes::table  ->  Node::structural  ->  Node::content
//
// - table guards slot allocation, every node's (dev, ino) key, and reference counts
//   reaching or leaving zero. It is held across openat() in open() and across the
//   rename in install(), so a lookup can never see a key that a rename has made stale.
// - structural (on a directory) serialises changes to its entries; (on a node) guards
//   rebinding the node to a different inode.
// - content guards `descriptor` for I/O. Readers hold it shared for the whole read, so
//   a replace swapping the descriptor waits for in-flight reads instead of tearing them.

class Node {
private:
	friend class Nodes;

	enum : u8 {
		FLAG_TYPE  = u8(1) << 0,
		FLAG_WRITE = u8(1) << 1,
	};

	char *name = nullptr; // Points into Nodes::names; strings could use a smaller reference type if the pool is local

	u64 dev = 0;
	u64 ino = 0;

	thread::Atomic<u64> size{0};

	thread::RWLock structural;
	thread::RWLock content;

	i32 descriptor = -1;

	thread::Atomic<u16> references{0};

	u8 flags = 0;

	u8 parent = 0;

	u8 _pad[16] = {}; // Padding can be used for additional metadata; could use SSO for names

public:
	// Only meaningful under the table lock (or content lock, for the descriptor).
	bool live() const;

	Type type() const;
	bool writable() const;

	// Binds a free slot. Caller holds the table lock and owns `fd`.
	void open(i32 fd, const struct stat &st, Type type, bool write, char *segment, u8 parent_index);

	// Releases the descriptor and resets the slot. Caller holds the table lock.
	void close();

	// Points the node at a new inode and returns the old descriptor for the caller to close.
	// Caller holds the table lock and the node's structural lock.
	i32 rebind(i32 fd, const struct stat &st);
};

// With a 4-byte RWLock this is exactly one cache line. The std::shared_mutex stand-in is larger.
// static_assert(sizeof(Node) == 64);

bool Node::live() const {
	return descriptor >= 0;
}

Type Node::type() const {
	return (flags & FLAG_TYPE) ? Type::FILE : Type::DIRECTORY;
}

bool Node::writable() const {
	return flags & FLAG_WRITE;
}

bool Node::open(i32 fd, const struct stat &st, Type t, bool write, char *segment, u8 parent_index) {
	name = segment;

	dev = u64(st.st_dev);
	ino = u64(st.st_ino);

	size.store(u64(st.st_size), thread::atomic::Order::RELEASE);

	descriptor = fd;

	flags = (t == Type::FILE ? FLAG_TYPE : 0) | (write ? FLAG_WRITE : 0);

	parent = parent_index;
}

void Node::close() {
	structural.wlock();
	content.wlock();

	if (live()) {
		name = nullptr;

		dev = 0;
		ino = 0;

		size.store(0, thread::atomic::Order::RELAXED);

		::close(descriptor);
		descriptor = -1;

		flags = 0;

		parent = 0;
	}

	content.wunlock();
	structural.wunlock();
}

i32 Node::rebind(i32 fd, const struct stat &st) {
	content.wlock();

	i32 retired = descriptor;
	descriptor = fd;

	dev = u64(st.st_dev);
	ino = u64(st.st_ino);

	size.store(u64(st.st_size), thread::atomic::Order::RELEASE);

	flags |= FLAG_WRITE; // O_TMPFILE and the fallback both open O_RDWR

	content.wunlock();
	return retired;
}

// Helpers

namespace {

i32 check_name(const char *name) {
	if (!name || !name[0]) {
		return -EINVAL;
	}

	u64 length = 0;
	for (; name[length]; ++length) {
		if (length == name::MAX_NAME_LENGTH) {
			return -ENAMETOOLONG;
		}
		if (name[length] == '/') {
			return -EINVAL;
		}
	}

	if (name[0] == '.' && (length == 1 || (length == 2 && name[1] == '.'))) {
		return -EINVAL;
	}

	return 0;
}

i32 write_all(i32 fd, const void *data, u64 bytes) {
	const u8 *bytes_in = static_cast<const u8 *>(data);
	u64 done = 0;

	while (done < bytes) {
		u64 chunk = bytes - done;
		if (chunk > (u64(1) << 30)) {
			chunk = u64(1) << 30;
		}

		ssize_t written = ::pwrite(fd, bytes_in + done, chunk, off_t(done));
		if (written < 0) {
			if (errno == EINTR) {
				continue;
			}
			return -errno;
		}
		if (written == 0) {
			return -EIO;
		}

		done += u64(written);
	}

	return 0;
}

// Sizes the staging name from the engine limit rather than the target, so a long target
// name can never push the temporary over NAME_MAX.
void staged_name(char (&out)[32], u32 sequence) {
	std::snprintf(out, sizeof out, ".descent.%d.%u.tmp", int(::getpid()), unsigned(sequence));
}

} // namespace

// Table

class Nodes {
public:
	static constexpr u64 MAX  = 256;
	static constexpr u8  NONE = 0; // slot 0 is never used, so a zeroed handle or parent is "none"

private:
	Node nodes[MAX];
	char names[MAX][name::MAX_NAME_LENGTH + 1] = {};

	thread::RWLock table;

	thread::Atomic<u32> sequence{0}; // staging names

	u8 find(u64 dev, u64 ino) const;
	u8 slot() const;

	// Binds slot `index` to `fd`. Table held.
	void bind(u8 index, i32 fd, const struct stat &st, Type type, bool write, const char *segment, u8 dir);

public:
	Node &operator[](u8 index) { return nodes[index]; }

	i32 root(const char *path, u8 &out);
	i32 open(u8 dir, const char *segment, Type type, bool write, u8 &out);

	void acquire(u8 index);
	void release(u8 index);

	// `bound`: a node to move onto the new file regardless of what is on disk now (File::replace).
	// `out`: receives a referenced node for the installed file, if non-null.
	i32 install(u8 dir, const char *segment, const void *data, u64 bytes, bool durable, u8 bound, u8 *out);

	// Installs over the node's own name in its own directory, and moves the node with it.
	i32 replace(u8 index, const void *data, u64 bytes, bool durable);

	i64 read(u8 index, u64 offset, void *out, u64 bytes);
	u64 size(u8 index) const;
};

static Nodes nodes;

u8 Nodes::find(u64 dev, u64 ino) const {
	for (u64 i = 1; i < MAX; ++i) {
		const Node &n = nodes[i];
		if (n.live() && n.ino == ino && n.dev == dev) {
			return u8(i);
		}
	}
	return NONE;
}

u8 Nodes::slot() const {
	for (u64 i = 1; i < MAX; ++i) {
		if (!nodes[i].live()) {
			return u8(i);
		}
	}
	return NONE;
}

void Nodes::bind(u8 index, i32 fd, const struct stat &st, Type type, bool write, const char *segment, u8 dir) {
	std::strncpy(names[index], segment, name::MAX_NAME_LENGTH);
	names[index][name::MAX_NAME_LENGTH] = '\0';

	nodes[index].open(fd, st, type, write, names[index], dir);
	nodes[index].references.store(1, thread::atomic::Order::RELAXED);

	if (dir != NONE) {
		nodes[dir].references.fetch_add(1, thread::atomic::Order::RELAXED); // children pin their parent
	}
}

i32 Nodes::root(const char *path, u8 &out) {
	i32 fd = ::open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (fd < 0) {
		return -errno;
	}

	struct stat st;
	if (::fstat(fd, &st) < 0) {
		i32 error = -errno;
		::close(fd);
		return error;
	}

	table.wlock();

	if (u8 found = find(u64(st.st_dev), u64(st.st_ino))) {
		nodes[found].references.fetch_add(1, thread::atomic::Order::RELAXED);
		table.wunlock();
		::close(fd);
		out = found;
		return 0;
	}

	u8 index = slot();
	if (index == NONE) {
		table.wunlock();
		::close(fd);
		return -ENFILE;
	}

	bind(index, fd, st, Type::DIRECTORY, false, "", NONE);

	table.wunlock();
	out = index;
	return 0;
}

i32 Nodes::open(u8 dir, const char *segment, Type type, bool write, u8 &out) {
	if (i32 error = check_name(segment)) {
		return error;
	}
	if (type == Type::DIRECTORY && write) {
		return -EINVAL;
	}

	Node &d = nodes[dir];
	if (dir == NONE || d.type() != Type::DIRECTORY) {
		return -ENOTDIR;
	}

	// O_NONBLOCK keeps openat from hanging on a FIFO; it is cleared once we know it's a regular file.
	int flags = O_CLOEXEC | O_NONBLOCK;
	flags |= type == Type::DIRECTORY ? (O_RDONLY | O_DIRECTORY) : (write ? O_RDWR : O_RDONLY);

	table.wlock();

	i32 fd = ::openat(d.descriptor, segment, flags);
	if (fd < 0) {
		i32 error = -errno;
		table.wunlock();
		return error;
	}

	struct stat st;
	i32 error = 0;
	if (::fstat(fd, &st) < 0) {
		error = -errno;
	} else if (type == Type::FILE && S_ISDIR(st.st_mode)) {
		error = -EISDIR;
	} else if (type == Type::FILE && !S_ISREG(st.st_mode)) {
		error = -EINVAL; // FIFOs, devices, sockets
	} else if (type == Type::FILE && ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) & ~O_NONBLOCK) < 0) {
		error = -errno;
	}

	if (error) {
		table.wunlock();
		::close(fd);
		return error;
	}

	if (u8 found = find(u64(st.st_dev), u64(st.st_ino))) {
		Node &n = nodes[found];

		if (n.type() != type) {
			table.wunlock();
			::close(fd);
			return type == Type::FILE ? -EISDIR : -ENOTDIR;
		}

		n.references.fetch_add(1, thread::atomic::Order::RELAXED);

		i32 spare = fd;
		if (write && !n.writable()) {
			// Upgrade the shared node to the read-write descriptor we just opened.
			n.content.wlock();
			spare = n.descriptor;
			n.descriptor = fd;
			n.flags |= Node::FLAG_WRITE;
			n.content.wunlock();
		}

		table.wunlock();
		::close(spare);
		out = found;
		return 0;
	}

	u8 index = slot();
	if (index == NONE) {
		table.wunlock();
		::close(fd);
		return -ENFILE;
	}

	bind(index, fd, st, type, write, segment, dir);

	table.wunlock();
	out = index;
	return 0;
}

void Nodes::acquire(u8 index) {
	// Only called by holders of an existing reference, so the count can't be zero here.
	nodes[index].references.fetch_add(1, thread::atomic::Order::RELAXED);
}

void Nodes::release(u8 index) {
	while (index != NONE) {
		Node &n = nodes[index];

		// Fast path: not the last reference, no table lock.
		u16 count = n.references.load(thread::atomic::Order::RELAXED);
		bool dropped = false;
		while (count > 1) {
			if (n.references.compare_exchange(count, u16(count - 1), thread::atomic::Order::RELEASE)) {
				dropped = true;
				break;
			}
		}
		if (dropped) {
			return;
		}

		// Possibly the last reference. Decide under the table lock, where open() also
		// takes references, so a node can't be found and closed at the same time.
		u8 parent = NONE;

		table.wlock();
		if (n.references.fetch_sub(1, thread::atomic::Order::ACQ_REL) == 1) {
			parent = n.parent;
			n.close();
		}
		table.wunlock();

		index = parent; // dropping a child releases its pin on the parent
	}
}

i32 Nodes::install(u8 dir, const char *segment, const void *data, u64 bytes, bool durable, u8 bound, u8 *out) {
	if (i32 error = check_name(segment)) {
		return error;
	}

	Node &d = nodes[dir];
	if (dir == NONE || d.type() != Type::DIRECTORY) {
		return -ENOTDIR;
	}

	// 1. Stage the contents with no locks held. The caller's reference keeps d.descriptor open,
	//    and directory descriptors are never swapped.
	char staged[32];
	bool linked = false;

#ifndef DESCENT_FS_NO_TMPFILE
	i32 fd = ::openat(d.descriptor, ".", O_TMPFILE | O_RDWR | O_CLOEXEC, 0644);
	if (fd < 0 && errno != EOPNOTSUPP && errno != EISDIR) {
		return -errno;
	}
#else
	i32 fd = -1;
#endif

	if (fd < 0) {
		// Filesystem without O_TMPFILE: a named, exclusive temporary instead.
		do {
			staged_name(staged, sequence.fetch_add(1, thread::atomic::Order::RELAXED));
			fd = ::openat(d.descriptor, staged, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0644);
		} while (fd < 0 && errno == EEXIST);

		if (fd < 0) {
			return -errno;
		}
		linked = true;
	}

	auto discard = [&](i32 error) {
		if (linked) {
			::unlinkat(d.descriptor, staged, 0);
		}
		::close(fd);
		return error;
	};

	if (i32 error = write_all(fd, data, bytes)) {
		return discard(error);
	}
	if (durable && ::fdatasync(fd) < 0) {
		return discard(-errno);
	}

	// 2. Publish.
	table.wlock();
	d.structural.wlock();

	auto abort = [&](i32 error) {
		d.structural.wunlock();
		table.wunlock();
		return discard(error);
	};

	struct stat previous;
	bool existed = ::fstatat(d.descriptor, segment, &previous, AT_SYMLINK_NOFOLLOW) == 0;
	if (!existed && errno != ENOENT) {
		return abort(-errno);
	}
	if (existed && S_ISDIR(previous.st_mode)) {
		return abort(-EISDIR);
	}

	// Which node, if any, follows the name onto the new file.
	u8 target = bound;
	if (target == NONE && existed) {
		u8 found = find(u64(previous.st_dev), u64(previous.st_ino));
		if (found != NONE && nodes[found].parent == dir && std::strcmp(nodes[found].name, segment) == 0) {
			target = found; // not a hard link reached through some other name
		}
	}

	u8 fresh = NONE;
	if (target == NONE && out) {
		fresh = slot(); // reserve before renaming, so running out of slots can't leave a half-finished install
		if (fresh == NONE) {
			return abort(-ENFILE);
		}
	}

	if (existed && S_ISREG(previous.st_mode)) {
		::fchmod(fd, previous.st_mode & 07777); // keep the file's permissions across saves
	}

	if (!linked) {
		char proc[32];
		std::snprintf(proc, sizeof proc, "/proc/self/fd/%d", int(fd));

		i32 result;
		do {
			staged_name(staged, sequence.fetch_add(1, thread::atomic::Order::RELAXED));
			result = ::linkat(AT_FDCWD, proc, d.descriptor, staged, AT_SYMLINK_FOLLOW);
		} while (result < 0 && errno == EEXIST);

		if (result < 0) {
			return abort(-errno);
		}
		linked = true;
	}

	if (::renameat(d.descriptor, staged, d.descriptor, segment) < 0) {
		return abort(-errno);
	}
	linked = false; // the staged name is gone; nothing left to clean up

	struct stat current;
	::fstat(fd, &current); // can't fail on a descriptor we hold

	i32 retired = -1;
	if (target != NONE) {
		Node &n = nodes[target];
		n.structural.wlock();
		retired = n.rebind(fd, current);
		n.structural.wunlock();
		fd = -1;

		if (out) {
			n.references.fetch_add(1, thread::atomic::Order::RELAXED);
		}
	} else if (fresh != NONE) {
		bind(fresh, fd, current, Type::FILE, true, segment, dir);
		target = fresh;
		fd = -1;
	}

	d.structural.wunlock();
	table.wunlock();

	if (retired >= 0) {
		::close(retired);
	}
	if (fd >= 0) {
		::close(fd);
	}

	if (durable && ::fsync(d.descriptor) < 0) {
		i32 error = -errno; // installed and visible, but the rename may not be durable
		if (out) {
			*out = target;
		}
		return error;
	}

	if (out) {
		*out = target;
	}
	return 0;
}

i64 Nodes::read(u8 index, u64 offset, void *out, u64 bytes) {
	Node &n = nodes[index];
	u8 *bytes_out = static_cast<u8 *>(out);
	u64 done = 0;

	n.content.rlock();

	while (done < bytes) {
		ssize_t got = ::pread(n.descriptor, bytes_out + done, bytes - done, off_t(offset + done));
		if (got < 0) {
			if (errno == EINTR) {
				continue;
			}
			i64 error = -errno;
			n.content.runlock();
			return error;
		}
		if (got == 0) {
			break; // end of file
		}
		done += u64(got);
	}

	n.content.runlock();
	return i64(done);
}

u64 Nodes::size(u8 index) const {
	return nodes[index].size.load(thread::atomic::Order::ACQUIRE);
}

i32 Nodes::replace(u8 index, const void *data, u64 bytes, bool durable) {
	Node &n = nodes[index];
	if (n.type() != Type::FILE) {
		return -EISDIR;
	}
	// name and parent never change while a reference is held.
	return install(n.parent, n.name, data, bytes, durable, index, nullptr);
}

// Interface

File::File(const File &other) : node(other.node) {
	if (node) {
		nodes.acquire(node);
	}
}

File::File(File &&other) noexcept : node(other.node) {
	other.node = 0;
}

File &File::operator=(File other) noexcept {
	u8 swap = node;
	node = other.node;
	other.node = swap;
	return *this;
}

File::~File() {
	nodes.release(node);
}

u64 File::size() const {
	return node ? nodes.size(node) : 0;
}

i64 File::read(u64 offset, void *out, u64 bytes) const {
	return node ? nodes.read(node, offset, out, bytes) : -EBADF;
}

i32 File::replace(const void *data, u64 bytes, Durability durability) {
	return node ? nodes.replace(node, data, bytes, durability == Durability::SYNC) : -EBADF;
}

Directory::Directory(const Directory &other) : node(other.node) {
	if (node) {
		nodes.acquire(node);
	}
}

Directory::Directory(Directory &&other) noexcept : node(other.node) {
	other.node = 0;
}

Directory &Directory::operator=(Directory other) noexcept {
	u8 swap = node;
	node = other.node;
	other.node = swap;
	return *this;
}

Directory::~Directory() {
	nodes.release(node);
}

i32 Directory::root(const char *path, Directory &out) {
	u8 index;
	if (i32 error = nodes.root(path, index)) {
		return error;
	}
	out = Directory(index);
	return 0;
}

i32 Directory::directory(const char *name, Directory &out) const {
	u8 index;
	if (i32 error = nodes.open(node, name, Type::DIRECTORY, false, index)) {
		return error;
	}
	out = Directory(index);
	return 0;
}

i32 Directory::file(const char *name, bool write, File &out) const {
	u8 index;
	if (i32 error = nodes.open(node, name, Type::FILE, write, index)) {
		return error;
	}
	out = File(index);
	return 0;
}

i32 Directory::install(const char *name, const void *data, u64 bytes, Durability durability, File *out) const {
	u8 index = 0;
	i32 error = nodes.install(node, name, data, bytes, durability == Durability::SYNC, 0, out ? &index : nullptr);
	if (out && index) {
		*out = File(index);
	}
	return error;
}

} // namespace descent::fs