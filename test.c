/* Test libuEv event library
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2015  Joachim Nilsson <troglobit()gmail!com>
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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		/* intptr_t */

#include "uev.h"

#define UNUSED(arg) arg __attribute__ ((unused))

typedef struct {
	int counter;
} my_t;

static int in, out;
static int period = 0;
static uev_t *watchdog;

static void lifetime_cb(uev_ctx_t *ctx, uev_t UNUSED(*w), void *arg, int UNUSED(events))
{
	fprintf(stderr, "\nLifetime exceeded, program completed successfully! (arg:%p)\n", arg);
	uev_exit(ctx);
}

/* The pipe watchdog, if it triggers we haven't received data in time. */
static void timeout_cb(uev_ctx_t *ctx, uev_t *UNUSED(w), void *arg, int UNUSED(events))
{
	watchdog = NULL;
	fprintf(stderr, "\nTimeout exceeded %p\n", arg);

	// uev_timer_stop(w); <-- No need to stop timers with period=0 :)
	uev_exit(ctx);
}

static void periodic_task(uev_ctx_t UNUSED(*ctx), uev_t UNUSED(*w), void UNUSED(*arg), int UNUSED(events))
{
	fprintf(stderr, "|");
}

static void signal_cb(uev_ctx_t *UNUSED(ctx), uev_t *w, void *UNUSED(arg), int UNUSED(events))
{
	fprintf(stderr, w->signo == SIGINT ? "^Cv" : "^\v");
}

static void pipe_read_cb(uev_ctx_t *UNUSED(ctx), uev_t UNUSED(*w), void UNUSED(*arg), int UNUSED(events))
{
	int cnt;
	char msg[50];

        /* Kick watchdog */
	if (watchdog)
		uev_timer_set(watchdog, 1000, 0);

	cnt = read(in, msg, sizeof(msg));
//	fprintf(stderr, "READ %.*s %d\n", cnt, msg, cnt);
	fprintf(stderr, "%.*s.%d ", cnt, msg, cnt);
}

static void pipe_write_cb(uev_ctx_t UNUSED(*ctx), uev_t *w, void *arg, int UNUSED(events))
{
	my_t *my  = arg;
	char *msg = "TESTING";

	if (write(out, msg, my->counter) < 0) {
                perror("\nFailed writing to pipe");
                return;
        }

//	fprintf(stderr, "WRITE %.*s %d\n", my->counter, msg, my->counter);
	fprintf(stderr, "%d ", my->counter);
	period = my->counter + 5;
	my->counter++;

	uev_timer_set(w, period * 100, 0);
}

int main(void)
{
	int fd[2];
	my_t my = { .counter = 1 };
	uev_t timeout, wdt, periodic, writer, reader, sigint_watcher, sigquit_watcher;
	uev_ctx_t ctx;

	/* Work load, one timer callback writes to a pipe periodically,
	 * and one I/O watcher that is called every time the pipe has
	 * some data. */
	if (pipe(fd) < 0)
		return 1;

	/*  */
	uev_init(&ctx);

	/* Signal watchers, Ctrl-C => SIGINT, Ctrl-\ => SIGQUIT */
	uev_signal_init(&ctx, &sigint_watcher, signal_cb, NULL, SIGINT);
	uev_signal_init(&ctx, &sigquit_watcher, signal_cb, NULL, SIGQUIT);

	/* Total program execution time */
	uev_timer_init(&ctx, &timeout, lifetime_cb, (void *)(intptr_t)2, 4000, 0);

	/* Main pipe worker and consumer */
	in  = fd[0];
	out = fd[1];
	uev_io_init(&ctx, &reader, pipe_read_cb, NULL, in, UEV_READ);
	uev_timer_init(&ctx, &writer, pipe_write_cb, &my, 400, 0);

	/* Watchdog for the above timer callback, if it doesn't wake up
	 * and write to the pipe within a given deadline it will bark. */
	uev_timer_init(&ctx, &wdt, timeout_cb, (void *)(intptr_t)1, 950, 0);
	watchdog = &wdt;

	/* Periodic background task */
	uev_timer_init(&ctx, &periodic, periodic_task, NULL, 200, 200);

	/* Start event loop */
	uev_run(&ctx, 0);

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
