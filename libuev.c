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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#include "libuev/uev.h"

static uev_io_t *new_watcher(uev_t *ctx, uev_type_t type, int fd, uev_dir_t dir, uev_cb_t *handler, void *data)
{
	uev_io_t *w;
	struct epoll_event ev;

	if (!ctx || !handler) {
		errno = EINVAL;
		return NULL;
	}

	w = (uev_io_t *)calloc(1, sizeof(*w));
	if (!w)
		return NULL;

	w->fd      = fd;
	w->dir     = dir;
	w->type    = type;
	w->handler = (void *)handler;
	w->data    = data;

	ev.events   = dir == UEV_DIR_OUTBOUND ? EPOLLOUT : EPOLLIN;
	ev.data.ptr = w;
	if (epoll_ctl(ctx->efd, EPOLL_CTL_ADD, fd, &ev) < 0) {
		free(w);
		return NULL;
	}

	TAILQ_INSERT_TAIL(&ctx->active_list, w, link);

	return w;
}

static int delete_watcher(uev_t *ctx, uev_io_t *w)
{
	uev_io_t *tmp;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	/* Check if already removed */
	TAILQ_FOREACH(tmp, &ctx->inactive_list, gc) {
		if (tmp == w) {
			errno = EALREADY;
			return -1;
		}
	}

	/* Remove from kernel */
	epoll_ctl(ctx->efd, EPOLL_CTL_DEL, w->fd, NULL);

	/* Mark as inactive */
	w->handler = NULL;
	TAILQ_INSERT_TAIL(&ctx->inactive_list, w, gc);

	return 0;
}

uev_io_t *uev_io_create(uev_t *ctx, uev_cb_t *handler, void *data, int fd, uev_dir_t dir)
{
	return new_watcher(ctx, UEV_FILE_TYPE, fd, dir, handler, data);
}

int uev_io_delete(uev_t *ctx, uev_io_t *w)
{
	return delete_watcher(ctx, w);
}

/**
 * Add a timer event
 * @param ctx     A valid libuev context
 * @param handler Timer callback
 * @param data    Optional callback argument
 * @param timeout Timeout in milliseconds before @param handler is called
 * @param period  For periodic timers this is the period time that @param timeout is reset to
 *
 * One-shot timers you likely set @param period to zero and only use
 * @param timeout.  For periodic timers you likely set @param timeout to
 * either zero, to call it as soon as the event loop starts, or to the
 * same value as @param period.  When the timer expires, the @param
 * handler is called, with the optional @param data argument.  A
 * non-periodic timer ends its life there, while a periodic task's
 * @param timeout is reset to the @param period and restarted.
 *
 * @return The new timer, or %NULL if invalid pointers or out or memory.
 */
uev_io_t *uev_timer_create(uev_t *ctx, uev_cb_t *handler, void *data, int timeout, int period)
{
	int fd;
	uev_io_t *w;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (fd < 0)
		return NULL;

	w = new_watcher(ctx, UEV_TIMER_TYPE, fd, UEV_DIR_INBOUND, handler, data);
	if (!w) {
		close(fd);
		return NULL;
	}

	if (uev_timer_set(ctx, w, timeout, period)) {
		delete_watcher(ctx, w);
		close(fd);
		return NULL;
	}

	return w;
}

/**
 * Reset or reschedule a timer
 */
int uev_timer_set(uev_t *ctx, uev_io_t *w, int timeout, int period)
{
	struct itimerspec time = {
		.it_value = {
			.tv_sec  =  timeout / 1000,
			.tv_nsec = (timeout % 1000) * 1000000
		},
		.it_interval = {
			.tv_sec  =  period  / 1000,
			.tv_nsec = (period  % 1000) * 1000000
		}
	};

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	w->timeout = timeout;
	w->period  = period;

	if (!ctx->running)
		return 0;

	return timerfd_settime(w->fd, 0, &time, NULL);
}

/**
 * Remove a timer event
 */
int uev_timer_delete(uev_t *ctx, uev_io_t *w)
{
	uev_timer_set(ctx, w, 0, 0);
	return delete_watcher(ctx, w);
}

/**
 * Create an application context
 */
uev_t *uev_ctx_create(void)
{
	int fd;
	uev_t *ctx;

	fd = epoll_create(1);
	if (fd < 0)
		return NULL;

	ctx = (uev_t *)calloc(1, sizeof(*ctx));
	if (!ctx) {
		close(fd);
		return NULL;
	}

	ctx->events = (struct epoll_event *)calloc(UEV_MAX_EVENTS, sizeof(struct epoll_event));
	if (!ctx->events) {
		close(fd);
		free(ctx);
		return NULL;
	}

	ctx->efd = fd;

	TAILQ_INIT(&ctx->active_list);
	TAILQ_INIT(&ctx->inactive_list);

	return ctx;
}

/**
 * Destroy an application context
 */
void uev_ctx_delete(uev_t *ctx)
{
        uev_io_t *w, *tmp;

        TAILQ_FOREACH_SAFE(w, &ctx->active_list, link, tmp) {
                TAILQ_REMOVE(&ctx->active_list, w, link);
                free(w);
        }

	close(ctx->efd);
	free(ctx->events);
	free(ctx);
}

/**
 * Run the application
 */
int uev_run(uev_t *ctx)
{
	int result = 0;
	uev_io_t *w, *tmp;

        if (!ctx) {
		errno = EINVAL;
                return 1;
	}

	/* Start the event loop */
	ctx->running = 1;

	/* Start all dormant timers */
	TAILQ_FOREACH(w, &ctx->active_list, link) {
		if (UEV_TIMER_TYPE == w->type)
			uev_timer_set(ctx, w, w->timeout, w->period);
	}

	while (ctx->running) {
		int i, nfds;

		while ((nfds = epoll_wait(ctx->efd, ctx->events, UEV_MAX_EVENTS, -1)) < 0) {
			if (EINTR == errno)
				continue; /* Signalled, try again */

			/* Error in poll. Cannot continue */
			result = 2;
			break;
		}

		for (i = 0; i < nfds; i++) {
			uev_io_t *w = (uev_io_t *)ctx->events[i].data.ptr;

			if (w->handler)
				w->handler((struct uev *)ctx, w, w->data);

			if (UEV_TIMER_TYPE == w->type) {
				int result;
				uint64_t exp;

				result = read(w->fd, &exp, sizeof(exp));
				if (result != sizeof(exp)) {
					/* Error in timerfd. Cannot continue */
					result = 3;
					ctx->running = 0;
				}

				if (!w->period)
					uev_timer_delete(ctx, w);
			}
		}

		/* Garbage collect */
		if (!TAILQ_EMPTY(&ctx->inactive_list)) {
			TAILQ_FOREACH_SAFE(w, &ctx->inactive_list, gc, tmp) {
				TAILQ_REMOVE(&ctx->inactive_list, w, gc);
				TAILQ_REMOVE(&ctx->active_list, w, link);
				free(w);
			}
		}
	}

	ctx->running = 0;

	return result;
}

/**
 * Terminate the application
 */
void uev_exit(uev_t *ctx)
{
	ctx->running = 0;
}


/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
