/* Verifies uev_io_set() on an active oneshot watcher */
#include "check.h"
#include <sys/socket.h>

static int fd = -1;
static int ev;

static void cb(uev_t *w, void *arg, int events)
{
	fd = w->fd;
	ev = events;
}

static void run(uev_ctx_t *ctx)
{
	fd = -1;
	ev = 0;
	fail_unless(uev_run(ctx, UEV_ONCE | UEV_NONBLOCK) == 0);
}

int main(void)
{
	int a[2], b[2];
	uev_ctx_t ctx;
	uev_t w;

	fail_unless(socketpair(AF_UNIX, SOCK_STREAM, 0, a) == 0);
	fail_unless(socketpair(AF_UNIX, SOCK_STREAM, 0, b) == 0);
	fail_unless(write(b[1], "b", 1) == 1);

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &w, cb, NULL, a[0], UEV_READ | UEV_ONESHOT) == 0);

	/* New descriptor */
	fail_unless(uev_io_set(&w, b[0], UEV_READ | UEV_ONESHOT) == 0);
	run(&ctx);
	fail_unless(fd == b[0]);

	/* New events, same descriptor */
	fail_unless(uev_io_set(&w, a[0], UEV_WRITE | UEV_ONESHOT) == 0);
	run(&ctx);
	fail_unless(fd == a[0]);
	fail_unless(ev & UEV_WRITE);

	/* Nothing new, only rearm */
	run(&ctx);
	fail_unless(fd == -1);
	fail_unless(uev_io_set(&w, a[0], UEV_WRITE | UEV_ONESHOT) == 0);
	run(&ctx);
	fail_unless(fd == a[0]);

	return uev_exit(&ctx);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
