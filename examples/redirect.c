/* Example of how to handle stdin redirect
 *
 * The problem is non-trivial since Linux epoll, which libuEv uses, does
 * not support I/O watchers for regular files or directories.
 *
 * This works:
 *                  echo foo | redirect
 * This doesn't:
 *                  redirect < foo.txt
 *
 * However, with a little bit of hacking libuEv can be made to handle
 * reading from stdin as a special case.
 */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "src/uev.h"

void process_stdin(uev_t *w, void *arg, int events)
{
	char buf[256];
	int len;

	if (UEV_ERROR == events) {
		warnx("Spurious problem with the stdin watcher, restarting.");
		uev_io_start(w);
	}

	len = read(w->fd, buf, sizeof(buf));
	if (len == -1) {
		warn("Error reading from stdin");
		return;
	}

	if (len == 0) {
		warnx("Connection closed.");
		return;
	}

	printf("Read %d bytes\n", len);
	if (write(STDOUT_FILENO, buf, len) != len)
		warn("Failed writing to stdout");
}

int main(int argc, char **argv)
{
	int ret;
	uev_t watcher;
	uev_ctx_t ctx;

	uev_init(&ctx);

	ret = uev_io_init(&ctx, &watcher, process_stdin, NULL, STDIN_FILENO, UEV_READ);
	if (ret)
		err(errno, "Failed setting up STDIN watcher");

	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  compile-command: "make redirect; echo hej > hej.txt; ./redirect < hej.txt"
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
