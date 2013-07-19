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

	w->rem.ent = w;
	w->fd      = fd;
	w->dir     = dir;
	w->handler = (void *)handler;
	w->data    = data;
	w->index   = -1;

	lListAppend(&ctx->io_list, w);

	return w;
}

int uev_io_delete(uev_t *ctx, uev_io_t *w)
{
	uev_io_rem_t *ent;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	lListForeachIn(ent, &ctx->io_delist) {
		if (ent->ent == w) {
			errno = EALREADY;
			return -1;	/* Already removed */
		}
	}

	w->handler = NULL;
	lListAppend(&ctx->io_delist, &w->rem);

	return 0;
}

/**
 * Add a timer event
 */
uev_timer_t *uev_timer_create(uev_t *ctx, int msec, uev_timer_cb_t handler, void *data)
{
	int diff;
	clock_t now;
	uev_timer_t *w, *ret, *ent;

	if (!ctx) {
		errno = EINVAL;
		return NULL;
	}

	w = (uev_timer_t *)malloc(sizeof(*ent));
	if (!w)
		return NULL;

	w->due     = msec;
	w->handler = (void *)handler;
	w->data    = data;

	if (0 == msec) {
		/* Zero delay is a special case: A oneshot workproc: Must go first */
		lListInsertFirst(&ctx->timer_list, w);

		return w;
	}

	/* Adjust timers.
	 * Jiffie wraparound is OK since we only care about diff time */
	now = ctx->use_next ? ctx->next_time : times(NULL);
	diff =   (now - ctx->base_time) / clock_tick  * 1000 +
                ((now - ctx->base_time) % clock_tick) * 1000 / clock_tick;

	ret = w;
	lListForeachIn(ent, &ctx->timer_list) {
		if (ent->due > diff)
			ent->due -= diff;
		else
			ent->due = 0;

		if (w && ent->due > msec) {
			lListInsert(&ctx->timer_list, w, ent);
			w = NULL;
		}
	}

	if (w)
		lListAppend(&ctx->timer_list, w);

	ctx->base_time = now;

	return ret;
}

/**
 * Remove a timer event
 */
int uev_timer_delete(uev_t *ctx, uev_timer_t *w)
{
	uev_timer_t *ent;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	lListForeachIn(ent, &ctx->timer_delist) {
		if (ent == w) {
			errno = EALREADY;
			return -1;	/* Already removed */
		}
	}

        lListRemove(&ctx->timer_list, w);
        lListAppend(&ctx->timer_delist, w);	/* Remember to free it (at a safe time) */

	return 0;
}

/**
 * Create an application context
 */
uev_t *uev_create(void)
{
	uev_t *ctx;

	ctx = (uev_t *)malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;

	lListInit(&ctx->io_list);
	lListInit(&ctx->timer_list);
	lListInit(&ctx->io_delist);
	lListInit(&ctx->timer_delist);

	ctx->base_time = times(NULL);
	ctx->next_time = 0;
	ctx->use_next  = false;

	ctx->exiting   = false;

	if (0 == clock_tick)
		clock_tick = sysconf(_SC_CLK_TCK);

	return ctx;
}

/**
 * Destroy an application context
 */
void uev_delete(uev_t *ctx)
{
	lListPurge(&ctx->io_list);
	lListPurge(&ctx->timer_list);
	free(ctx);
}

/**
 * Drive the timeouts ourself (from now on until an increment of -1)
 * This is intended for use in testing frameworks
 * Caveat emptor: Currently, interactions with the scheduler (agent) API is undefined.
 *                Not for use in uevs that use the agent scheduler
 * @param ctx UEVlication context object
 * @param msec Ms to advance. Use -1 to return to realtime
 */
void uev_run_tick(uev_t *ctx, int msec)
{
	// Obviously we are executing a callback right now (arent we always)
	// The time leap is taken into account before falling back into poll() below
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
static int do_run_pending(uev_t *ctx, int immediate)
{
	int          result, timeout = 0;
	clock_t      now;
	uev_timer_t *timer;

	/* Do due timers */
	if (lListHead(&ctx->timer_list)) {
		now     = ctx->use_next ? ctx->next_time : times(NULL);
		timeout =  (now - ctx->base_time) / clock_tick  * 1000
                        + ((now - ctx->base_time) % clock_tick) * 1000 / clock_tick;
		while ((timer = lListHead(&ctx->timer_list)) && timer->due <= timeout) {
			timer->handler((struct uev *)ctx, timer, timer->data);
			uev_timer_delete(ctx, timer);

			/* base_time changes for every uev_timer_create */
			timeout =  (now - ctx->base_time) / clock_tick  * 1000
                                + ((now - ctx->base_time) % clock_tick) * 1000 / clock_tick;
		}

		lListPurge(&ctx->timer_delist);	/* Reap timer zombies */
	}

	/* Calculate waiting time */
	if (ctx->exiting)
		timeout = 0;
	else if ((timer = lListHead(&ctx->timer_list)) && timer->due > timeout)
		timeout = timer->due - timeout;
	else if (NULL != timer)
		timeout = 0;
	else
		timeout = -1;

	/* Do pipe handlers */
	if (lListHead(&ctx->io_list) && !ctx->exiting) {
		int i = 0;
		struct pollfd polls[lListLength(&ctx->io_list)];
		uev_io_t *w;

		lListForeachIn(w, &ctx->io_list) {
			w->index = i;
			polls[i].fd = w->fd;
			polls[i].events = UEV_FHIN == w->dir ? POLLIN : POLLOUT;
			polls[i].revents = 0;
			i++;
		}

//                fprintf(stderr, "Polling %d sec\n", timeout);
		result = poll(polls, i, immediate ? 0 : timeout);
		if (result < 0) {
			if (EINTR == errno)
				return 0;

			if (EBADF == errno) {
				lListForeachIn(w, &ctx->io_list) {
					if (fcntl(w->fd, F_GETFD) == -1 && EBADF == errno)
						w->handler((struct uev *)ctx, w, w->fd, w->data);
				}
			} else {
				/* Error in poll. Cannot continue */
				assert(false);
				return;
			}
		} else if (result > 0) {
			lListForeachIn(w, &ctx->io_list) {
//                                fprintf(stderr, "I/O event!\n");
				if (w->handler && w->index >= 0 && polls[w->index].revents) {
					w->handler((struct uev *)ctx, w, w->fd, w->data);
				}
			}
		}

		if (lListHead(&ctx->io_delist)) {
			uev_io_rem_t *w;

			while ((w = lListHead(&ctx->io_delist))) {
				lListRemove(&ctx->io_delist, w);
				lListRemove(&ctx->io_list, w->ent);
				free(w->ent);
			}
		}
	} else if (timeout > 0) {
		if (!immediate)
			poll(NULL, 0, timeout);
	} else if (timeout == -1)
		timeout = -2;	/* Nothing more to do */

	return timeout;
}

/**
 * Process pending events
 */
int uev_run_pending(uev_t *ctx)
{
	return do_run_pending(ctx, 1);
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
 * Terminate the application
 */
void uev_exit(uev_t *ctx)
{
	ctx->exiting = true;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
