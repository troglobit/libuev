/* libuev - Asynchronous event loop library
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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#include "libuev.h"

static clock_t clock_tick = 0;

uev_io_t *uev_io_create(uev_t *ctx, int fd, uev_dir_t dir, uev_io_cb_t handler, void *data)
{
	uev_io_t *w;

	w = (uev_io_t *)malloc(sizeof(*w));
	if (!w)
		return NULL;

	w->fd      = fd;
	w->dir     = dir;
	w->handler = (void *)handler;
	w->data    = data;
	w->index   = -1;

	TAILQ_INSERT_TAIL(&ctx->io_list, w, link);

	return w;
}

int uev_io_delete(uev_t *ctx, uev_io_t *w)
{
	uev_io_t *tmp;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	/* Check if already removed */
	TAILQ_FOREACH(tmp, &ctx->io_delist, gc) {
		if (tmp == w) {
			errno = EALREADY;
			return -1;
		}
	}

	w->handler = NULL;
	TAILQ_INSERT_TAIL(&ctx->io_delist, w, gc);

	return 0;
}

/**
 * Add a timer event
 */
uev_timer_t *uev_timer_create(uev_t *ctx, int msec, uev_timer_cb_t handler, void *data)
{
	int diff;
	clock_t now;
	uev_timer_t *w, *ret, *tmp;

	if (!ctx) {
		errno = EINVAL;
		return NULL;
	}

	w = (uev_timer_t *)malloc(sizeof(*w));
	if (!w)
		return NULL;

	w->due     = msec;
	w->handler = (void *)handler;
	w->data    = data;

        /* Zero delay is a special case: A oneshot workproc: Must go first */
	if (0 == msec) {
		TAILQ_INSERT_HEAD(&ctx->timer_list, w, link);
		return w;
	}

	/* Adjust timers.
	 * Jiffie wraparound is OK since we only care about diff time */
	now  = ctx->use_next ? ctx->next_time : times(NULL);
	diff = TIME_DIFF_MSEC(now, ctx->base_time);

        /* To be returned below */
	ret = w;

        /* Update all timers and add this entry (ordered) to the list */
	TAILQ_FOREACH(tmp, &ctx->timer_list, link) {
		if (tmp->due > diff)
			tmp->due -= diff;
		else
			tmp->due = 0;

		if (w && tmp->due > msec) {
			TAILQ_INSERT_BEFORE(tmp, w, link);
			w = NULL;
		}
	}

	if (w)
		TAILQ_INSERT_TAIL(&ctx->timer_list, w, link);

        /* Update time base */
	ctx->base_time = now;

	return ret;
}

/**
 * Remove a timer event
 */
int uev_timer_delete(uev_t *ctx, uev_timer_t *w)
{
	uev_timer_t *tmp;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	/* Check if already removed */
	TAILQ_FOREACH(tmp, &ctx->timer_delist, link) {
		if (tmp == w) {
			errno = EALREADY;
			return -1;
		}
	}

        TAILQ_REMOVE(&ctx->timer_list, w, link);
        TAILQ_INSERT_TAIL(&ctx->timer_delist, w, link);

	return 0;
}

/**
 * Process pending events
 */
static int do_run_pending(uev_t *ctx, int immediate)
{
	int          i, result, diff = 0;
        size_t       num = 0;
	clock_t      now = 0;
        uev_io_t    *w;
	uev_timer_t *timer, *tmp;

	/* Do due timers */
	if (!TAILQ_EMPTY(&ctx->timer_list)) {
		now  = ctx->use_next ? ctx->next_time : times(NULL);
		diff = TIME_DIFF_MSEC(now, ctx->base_time);

                TAILQ_FOREACH(timer, &ctx->timer_list, link) {
                        if (timer->due > diff)
                                continue;

			timer->handler((struct uev *)ctx, timer, timer->data);
			uev_timer_delete(ctx, timer);

			/* base_time changes for every uev_timer_create(), which
                         * can be called in every timer->handler() above. */
                        diff = TIME_DIFF_MSEC(now, ctx->base_time);
		}

                /* Garbage collect */
                TAILQ_FOREACH_SAFE(timer, &ctx->timer_delist, link, tmp) {
                        TAILQ_REMOVE(&ctx->timer_delist, timer, link);
                        free(timer);
                }
	}

	/* Calculate waiting time */
	if (ctx->exiting)
		diff = 0;
	else if ((timer = TAILQ_FIRST(&ctx->timer_list)) && timer->due > diff)
		diff = timer->due - diff;
	else if (NULL != timer)
		diff = 0;
	else
		diff = -1;

	/* Do pipe handlers */
        TAILQ_FOREACH(w, &ctx->io_list, link)
                num++;

	if (!TAILQ_EMPTY(&ctx->io_list) && !ctx->exiting) {
		struct pollfd polls[num];

		/* Prepare all I/O watchers for poll() */
                i = 0;
		TAILQ_FOREACH(w, &ctx->io_list, link) {
			w->index = i;
			polls[i].fd = w->fd;
			polls[i].events = UEV_FHIN == w->dir ? POLLIN : POLLOUT;
			polls[i].revents = 0;
			i++;
		}

		while ((result = poll(polls, i, immediate ? 0 : diff)) < 0) {
			if (EINTR == errno)
				continue; /* Signalled, try again */

			if (EBADF == errno) {
				TAILQ_FOREACH(w, &ctx->io_list, link) {
					if (fcntl(w->fd, F_GETFD) == -1 && EBADF == errno)
						w->handler((struct uev *)ctx, w, w->fd, w->data);
				}
			} else {
				/* Error in poll. Cannot continue */
				assert(false);
				return;
			}
		}

		if (result > 0) {
			TAILQ_FOREACH(w, &ctx->io_list, link) {
				if (w->handler && w->index >= 0 && polls[w->index].revents)
					w->handler((struct uev *)ctx, w, w->fd, w->data);
			}
		}

                /* Garbage collect */
		if (!TAILQ_EMPTY(&ctx->io_delist)) {
			TAILQ_FOREACH(w, &ctx->io_delist, gc) {
				TAILQ_REMOVE(&ctx->io_delist, w, gc);
				TAILQ_REMOVE(&ctx->io_list, w, link);
				free(w);
			}
		}
	} else if (diff > 0) {
		if (!immediate)
			poll(NULL, 0, diff);
	} else if (diff == -1) {
		diff = -2;	/* Nothing more to do */
	}

	return diff;
}

/**
 * Run the application
 */
void uev_run(uev_t *ctx)
{
	int ret = 0;

        if (!ctx)
                return;

	while (!ctx->exiting && ret != -2)
		ret = do_run_pending(ctx, 0);

	ctx->exiting = false;
}

/**
 * Drive the timeouts ourself (from now on until an increment of -1)
 * This is intended for use in testing frameworks
 *
 * @param ctx  libuev context object
 * @param msec Milliseconds to advance the timer. Use -1 to return to realtime
 */
void uev_run_tick(uev_t *ctx, int msec)
{
	/* Obviously we are executing a callback right now (arent we always)
	 * The time leap is taken into account before falling back into poll() below */
	if (msec >= 0) {
		if (!ctx->use_next) {
			ctx->use_next  = true;
			ctx->next_time = times(NULL) + msec;
		} else {
			ctx->next_time += msec;
		}
	} else if (msec == -1) {
		/* Go back to realtime */
		ctx->use_next   = false;
		ctx->base_time += times(NULL) - ctx->next_time;
		ctx->next_time  = 0;
	}
}

/**
 * Process pending events
 */
int uev_run_pending(uev_t *ctx)
{
	return do_run_pending(ctx, 1);
}

/**
 * Terminate the application
 */
void uev_exit(uev_t *ctx)
{
	ctx->exiting = true;
}


/**
 * Create an application context
 */
uev_t *uev_ctx_create(void)
{
	uev_t *ctx;

	ctx = (uev_t *)malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;

	TAILQ_INIT(&ctx->io_list);
	TAILQ_INIT(&ctx->timer_list);
	TAILQ_INIT(&ctx->io_delist);
	TAILQ_INIT(&ctx->timer_delist);

	ctx->base_time = times(NULL);
	ctx->next_time = 0;
	ctx->use_next  = false;

	ctx->exiting   = false;

        /* Get system clock resolution, in ticks/second */
	if (0 == clock_tick)
		clock_tick = sysconf(_SC_CLK_TCK);

	return ctx;
}

/**
 * Destroy an application context
 */
void uev_ctx_delete(uev_t *ctx)
{
        uev_io_t    *io, *tmp_io;
        uev_timer_t *timer, *tmp_timer;

        TAILQ_FOREACH_SAFE(io, &ctx->io_list, link, tmp_io) {
                TAILQ_REMOVE(&ctx->io_list, io, link);
                free(io);
        }

        TAILQ_FOREACH_SAFE(timer, &ctx->timer_list, link, tmp_timer) {
                TAILQ_REMOVE(&ctx->timer_list, timer, link);
                free(timer);
        }

	free(ctx);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
