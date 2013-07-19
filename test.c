/* Test libuev event library
 *
 * Copyright (c) 2012  Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013  Joachim Nilsson <troglobit()gmail!com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>		/* intptr_t */

#include "libuev.h"

static int in, out;
static int period = 0;
static uev_timer_t *timer = NULL;

static int lifetime_cb(uev_t *ctx, uev_timer_t *w, void *data)
{
	fprintf(stderr, "\nLifetime exceeded %p\n", data);
	uev_exit(ctx);

        return 0;
}

/* The pipe watchdog, if it triggers we haven't received data in time. */
static int timeout_cb(uev_t *ctx, uev_timer_t *w, void *data)
{
	timer = NULL;
	fprintf(stderr, "\nTimeout exceeded %p\n", data);

	uev_timer_delete(ctx, w);
	uev_exit(ctx);

        return 0;
}

static int pipe_read_cb(uev_t *ctx, uev_io_t *w, int fd, void *data)
{
	int cnt;
	char msg[50];

        /* Kick watchdog */
	if (timer) {
		uev_timer_delete(ctx, timer);
		timer = uev_timer_create(ctx, 1000, timeout_cb, NULL);
	}

	cnt = read(in, msg, sizeof(msg));
//	fprintf(stderr, "READ %.*s %d\n", cnt, msg, cnt);
	fprintf(stderr, "%.*s.%d ", cnt, msg, cnt);

	return 0;
}

static int pipe_write_cb(uev_t *ctx, uev_timer_t *handle, void *data)
{
	int cnt = (int)(intptr_t)data;
	char *msg = "TESTING";

	write(out, msg, cnt);
//	fprintf(stderr, "WRITE %.*s %d\n", cnt, msg, cnt);
	fprintf(stderr, "%d ", cnt);
	period = cnt + 5;

	uev_timer_create(ctx, period * 100, pipe_write_cb, (void *)(intptr_t)(cnt + 1));

        return 0;
}

int main(void)
{
	int fd[2];
	uev_t *ctx = uev_create();

        /* Total program execution time */
	uev_timer_create(ctx, 4000, lifetime_cb, (void *)(intptr_t)2);

        /* Work load, one timer callback writes to a pipe periodically,
         * and one I/O watcher that is called every time the pipe has
         * some data. */
	pipe(fd);
	in  = fd[0];
	out = fd[1];
	uev_timer_create(ctx, 400,  pipe_write_cb, (void *)(intptr_t)1);
	uev_io_create(ctx, in, UEV_FHIN, pipe_read_cb, NULL);

        /* Watchdog for the above timer callback, if it doesn't wake up
         * and write to the pipe within a given deadline it will bark. */
	timer = uev_timer_create(ctx, 950, timeout_cb, (void *)(intptr_t)1);

        /* Start event loop */
	uev_run(ctx);

        /* Tear down event context */
	uev_delete(ctx);

	fprintf(stderr, "Period is %d must be 10: %s\n", period, 10 == period ? "OK" : "ERROR!");
        if (10 != period)
                return 1;
                
	return 0;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
