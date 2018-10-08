/* Verifies eventfd support
 *
 * Copyright (c) 2018  Joachim Nilsson <troglobit@gmail.com>
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
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define LAPS 10
pid_t pid;

static void cb(uev_t *w, void *arg, int events)
{
	static int num = LAPS;

	num--;
//	printf("Got event %d ...\n", LAPS - num);
	if (num == 0) {
		waitpid(pid, NULL, 0);
		uev_exit(w->ctx);
	}
}

int main(void)
{
	uev_ctx_t ctx;
	uev_t ev;
	int i;

	/* Initialize libuEv */
	uev_init(&ctx);

	/* Setup callbacks */
	uev_event_init(&ctx, &ev, cb, NULL);

	pid = fork();
	if (-1 == pid)
		err(1, "fork");

	/* Start event loop */
	if (pid > 0)
		return uev_run(&ctx, 0);

	for (i = 0; i < LAPS; i++) {
		uev_event_post(&ev);
		usleep(10000);
	}

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
