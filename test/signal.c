/* Multiple signals, segfault and fork() to test sub-processes.
 *
 * Copyright (c) 2015-2021  Joachim Wiberg <troglobit@gmail.com>
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

#define DO_SEGFAULT 1

typedef struct {
	pid_t pid;
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
	arg_t *event_arg = (arg_t *)foo;

	printf("child[%d] magic %d ptr %p, event %d event arg %p\n",
	       getpid(), arg->magic, arg->ptr, event, event_arg);

	arg->magic = 7331;
	arg->ptr   = &event_arg;

	/* Cause segfault! */
#if DO_SEGFAULT == 1
	event_arg->ptr = arg->ptr;
	printf("child: segfaulting failed!\n");
#endif

	return 42;
}

static void sig_cb(uev_t *w, void *cbarg, int events)
{
	arg_t *arg = (arg_t *)cbarg;

	switch (w->signo) {
	case SIGSEGV:
//		printf("Got SIGSEGV (%d) from PID %d\n", w->siginfo.ssi_signo, w->siginfo.ssi_pid);
		warnx("PID %d caused segfault.", getpid());
		exit(-1);
		break;

	case SIGCHLD:
		if (arg->pid != w->siginfo.ssi_pid)
			err(1, "wrong child exited pid %d vs ssi_pid %d",
			    arg->pid, w->siginfo.ssi_pid);

		warnx("Got SIGCHLD (%d), PID %d exited, bye.", w->siginfo.ssi_signo, arg->pid);
		break;

	default:
		err(1, "unhandled signal %d", w->siginfo.ssi_signo);
	}
}

static void work_cb(uev_t *w, void *cbarg, int events)
{
	arg_t *arg = (arg_t *)cbarg;
	pid_t pid;
	int rc = 0;

	pid = fork();
	if (-1 == pid)
		err(1, "fork");
	if (!pid)
		exit(callback(arg, 6137, NULL));

	arg->pid = pid;
	pid = waitpid(pid, &rc, 0);
	if (pid == -1)
		err(1, "waitpid");

	if (WIFEXITED(rc))
		printf("Child %d exited normally => %d\n", pid, WEXITSTATUS(rc));
	else if (WCOREDUMP(rc))
		printf("Child %d crashed%s\n", pid, DO_SEGFAULT
		       ? ", as expected, everything is OK."
		       : ".  This should not happen!");
	else
		printf("Child %d did not exit normally!\n", pid);
}

static void exit_cb(uev_t *w, void *arg, int events)
{
	printf("Process deadline reached, exiting!\n");
	uev_exit(w->ctx);
}

int main(void)
{
	uev_t deadline, sigsegv, sigchld, timeout;
	uev_ctx_t ctx;

	/* Initialize libuEv */
	uev_init(&ctx);

	/* Setup callbacks */
	uev_signal_init(&ctx, &sigsegv, sig_cb, NULL, SIGSEGV);
	uev_signal_init(&ctx, &sigchld, sig_cb, &arg, SIGCHLD);
	uev_timer_init(&ctx, &timeout, work_cb, &arg, 400, 0);
	uev_timer_init(&ctx, &deadline, exit_cb, NULL, 1000, 0);

	/* Start event loop */
	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
