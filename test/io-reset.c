#include <errno.h>
#include <sys/socket.h>
#include "check.h"

struct result {
	int calls;
	int fd;
	int events;
};

static void cb(uev_t *w, void *arg, int events)
{
	struct result *result = arg;

	result->calls++;
	result->fd = w->fd;
	result->events = events;
}

static void run_once(uev_ctx_t *ctx)
{
	fail_unless(uev_run(ctx, UEV_ONCE | UEV_NONBLOCK) == 0);
}

static void change_descriptor(void)
{
	uev_ctx_t ctx;
	uev_t watcher, oldwatcher;
	struct result result = { 0 }, oldresult = { 0 };
	int old[2], next[2];
	char byte;

	fail_unless(socketpair(AF_UNIX, SOCK_STREAM, 0, old) == 0);
	fail_unless(socketpair(AF_UNIX, SOCK_STREAM, 0, next) == 0);
	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &watcher, cb, &result, old[0], UEV_READ | UEV_ONESHOT) == 0);
	fail_unless(write(old[1], "a", 1) == 1);
	run_once(&ctx);
	fail_unless(result.calls == 1 && result.fd == old[0] && (result.events & UEV_READ));
	fail_unless(read(old[0], &byte, 1) == 1 && byte == 'a');

	fail_unless(uev_io_set(&watcher, next[0], UEV_READ | UEV_ONESHOT) == 0);
	fail_unless(write(next[1], "b", 1) == 1);
	run_once(&ctx);
	fail_unless(result.calls == 2 && result.fd == next[0] && (result.events & UEV_READ));
	fail_unless(read(next[0], &byte, 1) == 1 && byte == 'b');

	/* Rearm the same watcher repeatedly without duplicating list entries. */
	for (int i = 0; i < 8; i++) {
		fail_unless(uev_io_start(&watcher) == 0);
		fail_unless(write(next[1], "c", 1) == 1);
		run_once(&ctx);
		fail_unless(result.calls == 3 + i && result.fd == next[0]);
		fail_unless(read(next[0], &byte, 1) == 1 && byte == 'c');
		run_once(&ctx);
		fail_unless(result.calls == 3 + i);
	}

	/* Resetting does not close the old application-owned descriptor. */
	fail_unless(uev_io_init(&ctx, &oldwatcher, cb, &oldresult, old[0], UEV_READ) == 0);
	fail_unless(write(old[1], "d", 1) == 1);
	run_once(&ctx);
	fail_unless(oldresult.calls == 1 && oldresult.fd == old[0]);
	fail_unless(result.calls == 10);
	fail_unless(read(old[0], &byte, 1) == 1 && byte == 'd');
	fail_unless(uev_exit(&ctx) == 0);
	fail_unless(!uev_io_active(&watcher) && !uev_io_active(&oldwatcher));
	fail_unless(write(old[1], "e", 1) == 1 && read(old[0], &byte, 1) == 1);
	fail_unless(write(next[1], "f", 1) == 1 && read(next[0], &byte, 1) == 1);
	close(old[0]); close(old[1]); close(next[0]); close(next[1]);
}

static void change_events(int initial)
{
	uev_ctx_t ctx;
	uev_t watcher;
	struct result result = { 0 };
	int fd[2];

	fail_unless(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &watcher, cb, &result, fd[0], initial) == 0);
	fail_unless(uev_io_set(&watcher, fd[0], UEV_WRITE | UEV_ONESHOT) == 0);
	run_once(&ctx);
	fail_unless(result.calls == 1 && (result.events & UEV_WRITE));
	/* A regular-to-one-shot change must disable delivery until rearmed. */
	run_once(&ctx);
	fail_unless(result.calls == 1);
	fail_unless(uev_io_start(&watcher) == 0);
	run_once(&ctx);
	fail_unless(result.calls == 2 && (result.events & UEV_WRITE));

	/* Removing one-shot restores normal level-triggered delivery. */
	fail_unless(uev_io_set(&watcher, fd[0], UEV_WRITE) == 0);
	run_once(&ctx);
	run_once(&ctx);
	fail_unless(result.calls == 4);
	fail_unless(uev_io_stop(&watcher) == 0);
	run_once(&ctx);
	fail_unless(result.calls == 4);
	fail_unless(uev_io_start(&watcher) == 0);
	run_once(&ctx);
	fail_unless(result.calls == 5);
	fail_unless(uev_exit(&ctx) == 0);
	close(fd[0]); close(fd[1]);
}

static void failed_reset(void)
{
	uev_ctx_t ctx;
	uev_t watcher;
	struct result result = { 0 };
	int fd[2];

	fail_unless(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &watcher, cb, &result, fd[0], UEV_READ | UEV_ONESHOT) == 0);
	errno = 0;
	fail_unless(uev_io_set(&watcher, -1, UEV_READ | UEV_ONESHOT) == -1 && errno == EINVAL);
	fail_unless(!uev_io_active(&watcher));
	fail_unless(uev_io_set(&watcher, fd[0], UEV_WRITE | UEV_ONESHOT) == 0);
	run_once(&ctx);
	fail_unless(result.calls == 1 && (result.events & UEV_WRITE));
	fail_unless(uev_exit(&ctx) == 0);
	close(fd[0]); close(fd[1]);
}

int main(void)
{
	alarm(5);
	change_descriptor();
	change_events(UEV_READ | UEV_ONESHOT);
	change_events(UEV_READ);
	change_events(UEV_WRITE);
	failed_reset();
	alarm(0);
	return 0;
}
