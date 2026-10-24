// rengine::Rengine connects to the Wayland display on construction and
// disconnects on destruction. Neither case needs a compositor.

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
}

#include <harness.hpp>

#include <descent/rengine/rengine.hpp>

using namespace descent;

// With no compositor reachable, the Rengine reports false and is destroyed
// without disconnecting a null display (fixed in 2d06b17).
static void no_display() {
	unsetenv("WAYLAND_SOCKET");
	setenv("WAYLAND_DISPLAY", "descent-test-no-such-display", 1);

	rengine::Rengine r;
	test_check(!r, "connected although no display exists");
}

// Connected through WAYLAND_SOCKET, which wl_display_connect takes over as an
// already connected socket: the Rengine reports true, and its destruction
// closes the connection.
static void connected() {
	int fds[2];
	test_require(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, "socketpair");

	char fd[16];
	snprintf(fd, sizeof(fd), "%d", fds[1]);
	setenv("WAYLAND_SOCKET", fd, 1);

	{
		rengine::Rengine r;
		test_check(bool(r), "did not connect through WAYLAND_SOCKET");
	}

	char byte;
	test_check(read(fds[0], &byte, 1) == 0, "the connection was still open after destruction");
	close(fds[0]);
}

static constexpr test::Case cases[] = {
	{ "no_display", no_display },
	{ "connected",  connected },
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
