/* Verify that redirected stdin respects a single event-loop iteration. */
#include "check.h"
#include <signal.h>

struct input {
	int calls;
	int bytes;
	int eof;
};

static void cb(uev_t *w, void *arg, int events)
{
	struct input *input = arg;
	char ch;
	ssize_t len;

	fail_unless(events == UEV_READ);
	input->calls++;
	len = read(w->fd, &ch, 1);
	fail_unless(len >= 0);
	if (!len)
		input->eof++;
	else {
		fail_unless(ch == "abc"[input->bytes]);
		input->bytes++;
	}
}

static void redirected(int flags)
{
	struct input input = { 0 };
	uev_ctx_t ctx;
	uev_t watcher;
	FILE *file;
	int saved_stdin, i;

	saved_stdin = dup(STDIN_FILENO);
	fail_unless(saved_stdin >= 0);
	file = tmpfile();
	fail_unless(file != NULL);
	fail_unless(fwrite("abc", 1, 3, file) == 3);
	fail_unless(fflush(file) == 0);
	fail_unless(fseek(file, 0, SEEK_SET) == 0);
	fail_unless(dup2(fileno(file), STDIN_FILENO) == STDIN_FILENO);
	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &watcher, cb, &input, STDIN_FILENO, UEV_READ) == 0);

	if (flags & UEV_ONCE) {
		for (i = 1; i <= 3; i++) {
			fail_unless(uev_run(&ctx, flags) == 0);
			fail_unless(input.calls == i);
			fail_unless(input.bytes == i);
			fail_unless(input.eof == 0);
			fail_unless(lseek(STDIN_FILENO, 0, SEEK_CUR) == i);
		}
	}
	fail_unless(uev_run(&ctx, flags) == 0);
	fail_unless(input.calls == 4);
	fail_unless(input.bytes == 3);
	fail_unless(input.eof == 1);
	fail_unless(uev_run(&ctx, flags) == 0);
	fail_unless(input.calls == 4);
	fail_unless(uev_exit(&ctx) == 0);
	/* The watcher must never close the caller's input descriptor. */
	fail_unless(lseek(STDIN_FILENO, 0, SEEK_CUR) == 3);
	fail_unless(dup2(saved_stdin, STDIN_FILENO) == STDIN_FILENO);
	fail_unless(close(saved_stdin) == 0);
	fail_unless(fclose(file) == 0);
}

static void pipe_input(void)
{
	struct input input = { 0 };
	uev_ctx_t ctx;
	uev_t watcher;
	int fds[2], i;

	fail_unless(pipe(fds) == 0);
	fail_unless(write(fds[1], "abc", 3) == 3);
	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &watcher, cb, &input, fds[0], UEV_READ) == 0);
	for (i = 1; i <= 3; i++) {
		fail_unless(uev_run(&ctx, UEV_ONCE | UEV_NONBLOCK) == 0);
		fail_unless(input.calls == i);
		fail_unless(input.bytes == i);
		fail_unless(input.eof == 0);
	}
	fail_unless(uev_exit(&ctx) == 0);
	fail_unless(close(fds[0]) == 0);
	fail_unless(close(fds[1]) == 0);
}

int main(void)
{
	/* Bound a regression even if the event loop fails to yield. */
	alarm(5);
	redirected(UEV_ONCE);
	redirected(UEV_ONCE | UEV_NONBLOCK);
	redirected(0);
	pipe_input();
	alarm(0);

	return 0;
}
