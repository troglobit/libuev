/* Multiple signals, segfault and fork() to test sub-processes.
 *
 * Copyright (c) 2015  Joachim Nilsson <troglobit@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "check.h"
#include <err.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#define UNUSED(arg) arg __attribute__ ((unused))
#define DO_SEGFAULT 1

typedef struct {
	int magic;
	void *ptr;
} arg_t;

/* Stack is not shared between parent and child, only the heap. */
static arg_t arg = {
	.magic = 1337,
	.ptr = NULL
};

static int callback(arg_t *arg, int event, void *foo)
{
	arg_t *event_arg  = (arg_t *)foo;

	printf("child: got magic %d and ptr %p, with event %d and event arg %p\n",
	       arg->magic, arg->ptr, event, event_arg);

	arg->magic = 7331;
	arg->ptr   = &event_arg;

	/* Cause segfault! */
#if DO_SEGFAULT == 1
	event_arg->ptr = arg->ptr;
	printf("child: segfaulting failed!\n");
#endif

	return 42;
}

static void sigsegv_cb(uev_t *UNUSED(w), void *UNUSED(arg), int UNUSED(events))
{
	warnx("PID %d caused segfault.", getpid());
	exit(-1);
}

static void sigchld_cb(uev_t *UNUSED(w), void *UNUSED(arg), int UNUSED(events))
{
	pid_t pid = waitpid(-1, NULL, WNOHANG);

	if (-1 != pid)
		warnx("PID %d exited, bye.", pid);
}

static void work_cb(uev_t *UNUSED(w), void *arg, int UNUSED(events))
{
	int    status = 0;
	pid_t  pid;

	pid = fork();
	if (-1 == pid)
		err(1, "fork");
	if (!pid) {
		status = callback(arg, 6137, NULL);
		exit(status);
	}

	if (waitpid(pid, &status, 0) == -1)
		err(1, "waitpid");

	if (WIFEXITED(status))
		printf("Child exited normally => %d\n", WEXITSTATUS(status));
	else if (WCOREDUMP(status))
		printf("Child crashed! %s\n", DO_SEGFAULT
		       ? "As expected, everything is OK."
		       : "This should not happen!");
	else
		printf("Child did not exit normally!\n");
}

static void exit_cb(uev_t *w, void *UNUSED(arg), int UNUSED(events))
{
	printf("Process deadline reached, exiting!\n");
	uev_exit(w->ctx);
}

int main(void)
{
	uev_t sigsegv_watcher, sigchld_watcher, timeout_watcher, deadline_watcher;
	uev_ctx_t ctx;

	/* Initialize libuEv */
	uev_init(&ctx);

	/* Setup callbacks */
	uev_signal_init(&ctx, &sigsegv_watcher, sigsegv_cb, NULL, SIGSEGV);
	uev_signal_init(&ctx, &sigchld_watcher, sigchld_cb, NULL, SIGCHLD);
	uev_timer_init(&ctx, &timeout_watcher, work_cb, &arg, 400, 0);
	uev_timer_init(&ctx, &deadline_watcher, exit_cb, NULL, 1000, 0);

	/* Start event loop */
	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
